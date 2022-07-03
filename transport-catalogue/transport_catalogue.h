#pragma once

#include "domain.h"

#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <map>

namespace transport {

class TransportCatalogue {

public:
    void AddStop(std::string_view name, const geo::Coordinates coords);

    const Stop* FindStop(std::string_view name);

    void SetDistance(const Stop*, const Stop*, int);

    void AddBus(std::string_view, std::deque<Stop*>&&, Stop* last_stop = nullptr);

    StopInfo GetStopInfo(std::string_view stop_name) const;

    BusInfo GetBusInfo(std::string_view bus_name) const;

    double GetDistance(Stop* from, Stop* to) const;

    const std::unordered_map<std::string_view, Bus*>& GetBussesIndex() const;

    const std::deque<Stop>& GetStops() const { return stops_; }

    const std::deque<transport::Bus>& GetBuses() const { return buses_; }

    void PrintTest();

private:
    std::deque<Stop> stops_;
    std::unordered_map<StopsPtr, int, StopsPtrHash> distances_;
    std::unordered_map<std::string_view, Stop*> stops_indx_;
    std::deque<transport::Bus> buses_;
    std::unordered_map<std::string_view, Bus*> buses_indx_;
    std::unordered_map<Stop*, std::unordered_set<Bus*>> stops_buses_indx_;
};

} //namespace transport
