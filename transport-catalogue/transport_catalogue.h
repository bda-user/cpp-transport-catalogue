#pragma once

#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>

#include "geo.h"

namespace transport {

struct Stop {
    std::string name{""};
    Coordinates coordinates{0, 0};
};

struct StopsPtr {
    Stop* ptr1 = nullptr;
    Stop* ptr2 = nullptr;

private:
    auto AsTuple() const {
        return std::tie(ptr1, ptr2);
    }

public:
    bool operator==(const StopsPtr& other) const {
        return AsTuple() == other.AsTuple();
    }
};

struct StopsPtrHash {
    size_t operator() (const StopsPtr& sp) const {
        return static_cast<size_t>(ptr_hasher_(sp.ptr1) +
                                   ptr_hasher_(sp.ptr2) * 37);
    }
private:
    std::hash<Stop*> ptr_hasher_;
};

struct Bus {
    std::string name{""};
    std::deque<Stop*> stops{};
};

struct StopInfo {
    std::string name;
    Stop* ptr;
    std::unordered_set<Bus*>* buses;
};

struct BusInfo {
    std::string name;
    Bus* ptr;
};

class TransportCatalogue {

public:
    void AddStop(std::string_view name, Coordinates coords);

    const Stop* FindStop(std::string_view name);

    void SetDistance(const Stop*, const Stop*, int);

    void AddBus(std::string_view, std::deque<Stop*>&&);

    StopInfo GetStopInfo(std::string_view stop_name);

    BusInfo GetBusInfo(std::string_view bus_name);

    int GetDistance(Stop* from, Stop* to);

    void PrintTest();

private:
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> stops_indx_;
    std::unordered_map<StopsPtr, int, StopsPtrHash> distances_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Bus*> buses_indx_;
    std::unordered_map<Stop*, std::unordered_set<Bus*>> stops_buses_indx_;
};

}
