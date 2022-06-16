#pragma once

#include <string>
#include <deque>
#include <unordered_set>

#include "geo.h"

namespace transport {

struct Stop {
    std::string name{""};
    geo::Coordinates coordinates{0, 0};
};

struct StopsPtr {
    Stop* ptr1 = nullptr;
    Stop* ptr2 = nullptr;

    bool operator==(const StopsPtr& other) const;

private:
    auto AsTuple() const;
};

struct StopsPtrHash {
    size_t operator() (const StopsPtr& sp) const;

private:
    std::hash<Stop*> ptr_hasher_;
};

struct Bus {
    std::string name{""};
    std::deque<Stop*> stops{};
    Stop* last_stop{nullptr};
};

struct StopInfo {
    std::string name;
    Stop* ptr;
    std::unordered_set<transport::Bus*>* buses;
};

struct BusInfo {
    std::string name;
    Bus* ptr;
};

struct BusStat {
    double curvature = 0.0;
    int route_length = 0;
    int stop_count = 0;
    int unique_stop_count = 0;
};

using BusPtr = Bus*;

} // namespace transport
