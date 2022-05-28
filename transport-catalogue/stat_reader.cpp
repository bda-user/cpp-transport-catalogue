#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include "stat_reader.h"
#include "geo.h"

namespace transport {

std::string StatReader::FormatStopInfo(std::string_view name) {
    StopInfo stop_info = catalogue_.GetStopInfo(name);
    std::string res("Stop " + stop_info.name + ": ");
    if(stop_info.ptr == nullptr) {
        res += "not found";
    } else
    if(stop_info.buses == nullptr) {
        res += "no buses";
    } else {
        res += "buses";
        std::vector<std::string_view> buses;
        for(auto bus : *stop_info.buses) {
            buses.push_back(bus->name);
        }
        std::sort(buses.begin(), buses.end());
        for(auto bus : buses) {
            res += " " + std::string(bus);
        }
    }
    return res;
}

std::string StatReader::FormatBusInfo(std::string_view name) {
    BusInfo bus_info = catalogue_.GetBusInfo(name);
    std::string res("Bus " + bus_info.name + ": ");
    if(bus_info.ptr == nullptr) {
        return res + "not found";
    }

    auto& stops = bus_info.ptr->stops;
    int stop_count = stops.size();
    std::unordered_set<Stop*> unic_stops;
    double route_length = 0.0;
    int route_length_distance = 0;
    auto from_it = stops.begin();
    auto to_it = from_it + 1;

    while(to_it != stops.end()) {
        unic_stops.insert(*from_it);
        auto distance = ComputeDistance((*from_it)->coordinates, (*to_it)->coordinates);
        route_length += distance;
        auto distance2 = catalogue_.GetDistance(const_cast<Stop*>(*from_it),
                                     const_cast<Stop*>(*to_it));
        route_length_distance += distance2;
        ++from_it;
        ++to_it;
    }

    std::ostringstream osstream;
    osstream << std::setprecision(6);
    osstream << route_length_distance/route_length;

    return res + std::to_string(stop_count) + " stops on route, " +
            std::to_string(unic_stops.size()) + " unique stops, " +
            std::to_string(route_length_distance) + " route length, " +
            osstream.str() + " curvature";
}

}
