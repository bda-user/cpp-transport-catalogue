#include "request_handler.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

using namespace std::literals;

namespace transport {

const std::unordered_set<transport::BusPtr>* RequestHandler::GetBusesByStop(
        const std::string_view& stop_name) const {
    StopInfo stop_info = db_.GetStopInfo(stop_name);
    return std::move(stop_info.buses);
}

std::optional<transport::BusStat> RequestHandler::GetBusStat(
        const std::string_view& bus_name) const {
    BusStat bus_stat;

    BusInfo bus_info = db_.GetBusInfo(bus_name);

    if(bus_info.ptr == nullptr) {
        return {};
    }

    auto& stops = bus_info.ptr->stops;
    int stop_count = stops.size();
    std::unordered_set<Stop*> unic_stops;
    double route_length = 0.0, route_length_distance = 0.0;
    auto from_it = stops.begin();
    auto to_it = from_it + 1;

    while(to_it != stops.end()) {
        unic_stops.insert(*from_it);
        route_length += ComputeDistance((*from_it)->coordinates, (*to_it)->coordinates);
        route_length_distance += db_.GetDistance(
                                    const_cast<Stop*>(*from_it),
                                    const_cast<Stop*>(*to_it));
        ++from_it;
        ++to_it;
    }

    bus_stat.stop_count = stop_count;
    bus_stat.unique_stop_count = unic_stops.size();
    bus_stat.route_length = route_length_distance;
    bus_stat.curvature = route_length_distance/route_length;

    return std::optional<transport::BusStat>{std::move(bus_stat)};
}

SphereProjector RequestHandler::MakeSphereProjector(const renderer::RenderSettings& render_settings) {
    using namespace svg;
    using namespace std;

    vector<int> bus_stop_cnts;
    vector<geo::Coordinates> geo_coords;

    // sorting bus name
    const auto& buses = db_.GetBussesIndex();
    for(const auto& [_, bus] : buses) {
        if(bus->stops.size() == 0) continue;
        buses_.push_back(bus);
    }

    sort(buses_.begin(), buses_.end(),
         [] (BusPtr const& lhs, BusPtr const& rhs) { return lhs->name < rhs->name; });

    for(const auto bus : buses_) {
        for(const auto stop : bus->stops) {
            geo_coords.push_back(stop->coordinates);
            stops_.insert(stop);
        }
        bus_stop_cnts.push_back(bus->stops.size());
    }

    // Project & set screen coords
    const SphereProjector proj{geo_coords.begin(), geo_coords.end(),
                render_settings.width,
                render_settings.height,
                render_settings.padding
        };

    // fill Routs
    int j = 0;
    for(auto cnt : bus_stop_cnts) {
        vector<Point> route;
        for(int i = 0; i < cnt; ++i) {
            route.push_back(proj(move(geo_coords[j])));
            ++j;
        }
        layers_.routs.push_back(move(route));
    }

    return proj;
}

svg::Document RequestHandler::RenderMap(const renderer::RenderSettings& render_settings) {
    using namespace svg;
    using namespace std;

    // fill Routs
    SphereProjector proj = MakeSphereProjector(render_settings);

    // fill Labels
    for(const auto bus : buses_) {
        renderer::RouteLabel bus_label;
        bus_label.name = bus->name;
        bus_label.first = proj(bus->stops[0]->coordinates);
        if(bus->last_stop != nullptr &&
                bus->stops[0]->name != bus->last_stop->name) {
            bus_label.second = proj(bus->last_stop->coordinates);
        }
        layers_.labels.push_back(std::move(bus_label));
    }

    // fill Stops

    for(const auto stop : stops_) {
        layers_.stops.push_back(renderer::RouteStop{stop->name, proj(stop->coordinates)});
    }

    sort(layers_.stops.begin(), layers_.stops.end(),
         [] (renderer::RouteStop const& lhs, renderer::RouteStop const& rhs) { return lhs.name < rhs.name; });

    return renderer_.RenderMap(layers_, render_settings);
}

} //namespace transport
