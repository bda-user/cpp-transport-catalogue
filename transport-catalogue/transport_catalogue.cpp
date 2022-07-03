#include <iostream>

#include "transport_catalogue.h"

namespace transport {

void TransportCatalogue::AddStop(std::string_view name, const geo::Coordinates coords){
    stops_.push_back({std::string(name), coords});
    auto& last_stop = stops_.back();
    stops_indx_.insert({last_stop.name, &last_stop});
}

const Stop* TransportCatalogue::FindStop(std::string_view name) {
    if(auto it = stops_indx_.find(name); it != stops_indx_.end()) {
        return it->second;
    }
    return nullptr;
}

void TransportCatalogue::SetDistance(const Stop* from, const Stop* to, int distance) {
    distances_.insert({{const_cast<Stop*>(from), const_cast<Stop*>(to)}, distance});
}

void TransportCatalogue::AddBus(std::string_view name, std::deque<Stop*>&& stops, Stop* last_stop) {
    buses_.push_back({std::string(name), std::move(stops), last_stop});
    auto& last_bus = buses_.back();
    buses_indx_.insert({last_bus.name, &last_bus});
    for(auto& stop : last_bus.stops) {
        stops_buses_indx_[stop].insert(&last_bus);
    }
}

StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
    std::string name(stop_name);
    if(auto it = stops_indx_.find(stop_name); it != stops_indx_.end()) {
        if(auto itb = stops_buses_indx_.find(it->second); itb != stops_buses_indx_.end()) {
            return {std::move(name), it->second, const_cast<std::unordered_set<transport::Bus*>*>(&itb->second)};
        }
        return {std::move(name), it->second, nullptr};
    } else {
        return {std::move(name), nullptr, nullptr};
    }
}

BusInfo TransportCatalogue::GetBusInfo(std::string_view bus_name)  const {
    std::string name(bus_name);
    if(auto it = buses_indx_.find(bus_name); it != buses_indx_.end()) {
        return {std::move(name), it->second};
    } else {
        return {std::move(name), nullptr};
    }
}

double TransportCatalogue::GetDistance(Stop* from, Stop* to)  const {
    if(auto it = distances_.find({from, to}); it != distances_.end()) {
        return it->second;
    } else
    if(auto it = distances_.find({to, from}); it != distances_.end()) {
        return it->second;
    }
    return 0.0;
}

const std::unordered_map<std::string_view, Bus*>& TransportCatalogue::GetBussesIndex() const {
    return buses_indx_;
}

void TransportCatalogue::PrintTest(){
    for(auto& stop : stops_) {
        std::cout << stop.name << ": " <<
                     stop.coordinates.lat << ", " <<
                     stop.coordinates.lng << std::endl;
    }
    std::cout << std::endl;

    for(auto [name, _] : stops_indx_) {
        std::cout << name << ", ";
    }
    std::cout << std::endl << std::endl;

    for(auto [_, distance] : distances_) {
        std::cout << distance << ", ";
    }
    std::cout << std::endl << std::endl;

    for(auto& bus : buses_) {
        std::cout << bus.name << ": ";
        for(auto& stop : bus.stops) {
             std::cout << stop->name << ", ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    for(auto [name, _] : buses_indx_) {
        std::cout << name << ", ";
    }
    std::cout << std::endl << std::endl;

    for(auto& [stop, buses] : stops_buses_indx_) {
        std::cout << const_cast<Stop*>(stop)->name << ": ";
        for(auto& bus : buses) {
             std::cout << bus->name << ", ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

}

}
