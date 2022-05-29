#include "input_reader.h"
#include "stat_reader.h"

namespace transport {

namespace detail {

std::pair<std::string_view, std::string_view> Split(std::string_view line, char by) {
    size_t pos = line.find(by);
    std::string_view left = SpaceTrim(line.substr(0, pos));

    if (pos < line.size() && pos + 1 < line.size()) {
        return {left, SpaceTrim(line.substr(pos + 1))};
    } else {
        return {left, std::string_view()};
    }
}

std::string_view SpaceTrim(std::string_view line) {
    while (!line.empty() && isspace(line[0])) {
        line.remove_prefix(1);
    }
    while (!line.empty() && isspace(line.back())) {
        line.remove_suffix(1);
    }
    return line;
}

std::pair<std::string_view, int> ParseDistance(std::string_view line_distance) {
    auto [distance, right] = Split(line_distance, ' ');
    auto [_, stop] = Split(right, ' ');
    return {stop, std::stoi(distance.data())};
}

}
// end namespace detail

void InputReader::LoadStop(std::string_view line, QueriesDistances& queries_distances) {
    using namespace detail;
    auto [left, coords] = Split(line, ':');
    auto [_, name] = Split(left, ' ');
    auto [lat, tail] = Split(coords, ',');
    auto [lng, distance] = Split(tail, ',');
    catalogue_.AddStop(name, {std::stod(lat.data()), std::stod(lng.data())});
    if(!distance.empty()) {
        queries_distances.push_back({std::string(name), std::string(distance)});
    }
}

void InputReader::LoadDistance(QueriesDistances& queries_distances) {
    for(auto& [stop_name, query_distances] : queries_distances) {
        auto stop1 = catalogue_.FindStop(stop_name);
        if(stop1 == nullptr) {
            continue;
        }
        std::string_view right_src(query_distances);
        while(!right_src.empty()){
            auto [left, right] = detail::Split(right_src, ',');
            right_src = right;
            auto [stop, distance] = detail::ParseDistance(left);
            auto stop2 = catalogue_.FindStop(stop);
            if(stop2 != nullptr) {
                catalogue_.SetDistance(stop1, stop2, distance);
            }
        }
    }
}

void InputReader::LoadBuses(std::deque<std::string>& queries_buses) {
    using namespace detail;
    for(auto& query_bus : queries_buses) {
        auto [bus, stops] = Split(query_bus, ':');
        auto [_, bus_name] = Split(bus, ' ');

        char delimeter = '>';
        size_t pos = stops.find(delimeter);
        if(pos > stops.size()) {
            delimeter = '-';
        }

        std::deque<Stop*> bus_stops;
        auto [left, right] = Split(stops, delimeter);
        auto stop = left;
        auto right_src = right;
        while(!right_src.empty()){
            auto stop_name = catalogue_.FindStop(stop);
            if(stop_name != nullptr) {
                bus_stops.push_back(const_cast<Stop*>(stop_name));
            }
            auto [left, right] = Split(right_src, delimeter);
            stop = left;
            right_src = right;
        }
        auto stop_name = catalogue_.FindStop(stop);
        if(stop_name != nullptr) {
            bus_stops.push_back(const_cast<Stop*>(stop_name));
        }

        if(delimeter == '-') {
            auto rit = bus_stops.end();
            while(--rit > bus_stops.begin()) {
                bus_stops.push_back(*(rit - 1));
            }
        }

        catalogue_.AddBus(bus_name, std::move(bus_stops));
    }
}

void InputReader::Load(std::istream& input) {
    int line_count = 0;
    std::deque<std::string> queries_buses;
    QueriesDistances queries_distances;
    for (std::string line; getline(input, line) && !line.empty();) {
        if(line_count == 0) {
            line_count = std::stoi(line);
        } else
        if(line_count > 0){
            if(line[0] == 'S') {
                LoadStop(line, queries_distances);
            } else {
                queries_buses.push_back(line);
            }
            if(--line_count == 0){
                break;
            }
        }
    }
    LoadDistance(queries_distances);
    LoadBuses(queries_buses);
}

}
