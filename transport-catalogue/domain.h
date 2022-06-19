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
    Stop* from_ptr = nullptr;
    Stop* to_ptr = nullptr;

    bool operator==(const StopsPtr& other) const;

private:
    auto AsTuple() const;
};

struct StopsPtrHash {
    size_t operator() (const StopsPtr& sp) const;

private:
    std::hash<Stop*> ptr_hasher_;
};

/*
"last_stop" need for NoRound Bus. NoRound Bus has route: A - B - C, last_stop = C
Full route: A - B - C - B - C
In map.svg need drow stops A & C if: Bus = NoRound && A != C
See json_reader.cpp line 56
*/
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
