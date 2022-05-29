#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "input_reader.h"
#include "stat_reader.h"
#include "geo.h"

using namespace std::literals;

namespace transport {

void StatReader::LoadQueries(std::istream& input) {
    int line_count = 0;
    for (std::string line; getline(input, line) && !line.empty();) {
        if(line_count == 0) {
            line_count = std::stoi(line);
        } else
        if(line_count > 0){
            queries_.push_back(line);
            if(--line_count == 0){
                break;
            }
        }
    }
}

void StatReader::ExecQueries(std::ostream& output) {
    for(auto& line : queries_) {
        auto [query, name] = detail::Split(line, ' ');
        if(query == "Bus"s) {
            output << FormatBusInfo(name) << std::endl;
        } else {
            output << FormatStopInfo(name) << std::endl;
        }
    }
}

std::string StatReader::FormatStopInfo(std::string_view name) {
    std::ostringstream osstream;
    StopInfo stop_info = catalogue_.GetStopInfo(name);
    osstream << "Stop "s << stop_info.name << ": "s;
    if(stop_info.ptr == nullptr) {
        osstream << "not found"s;
    } else
    if(stop_info.buses == nullptr) {
        osstream << "no buses"s;
    } else {
        osstream << "buses"s;
        std::vector<std::string_view> buses;
        for(auto bus : *stop_info.buses) {
            buses.push_back(bus->name);
        }
        std::sort(buses.begin(), buses.end());
        for(auto bus : buses) {
            osstream << " "s << bus;
        }
    }
    return osstream.str();
}

std::string StatReader::FormatBusInfo(std::string_view name) {
    std::ostringstream osstream;
    osstream << std::setprecision(6);
    BusInfo bus_info = catalogue_.GetBusInfo(name);

    osstream << "Bus "s << bus_info.name << ": "s;
    if(bus_info.ptr == nullptr) {
        osstream << "not found"s;
        return osstream.str();
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
        route_length_distance += catalogue_.GetDistance(
                                    const_cast<Stop*>(*from_it),
                                    const_cast<Stop*>(*to_it));
        ++from_it;
        ++to_it;
    }

    osstream << stop_count << " stops on route, "s <<
                unic_stops.size() << " unique stops, "s <<
                route_length_distance << " route length, "s <<
                route_length_distance/route_length << " curvature"s;

    return osstream.str();
}

}
