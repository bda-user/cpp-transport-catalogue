#include "input_reader.h"
#include "stat_reader.h"

namespace transport {

namespace detail {

std::pair<std::string_view, std::string_view> Split(std::string_view line, char by) {
    size_t pos = line.find(by);
    std::string_view left = line.substr(0, pos);

    if (pos < line.size() && pos + 1 < line.size()) {
        return {left, line.substr(pos + 1)};
    } else {
        return {left, std::string_view()};
    }
}

std::string_view Lstrip(std::string_view line) {
    while (!line.empty() && isspace(line[0])) {
        line.remove_prefix(1);
    }
    while (!line.empty() && line.back() == ' ') {
        line.remove_suffix(1);
    }
    return line;
}

std::pair<std::string_view, int> ParseDistance(std::string_view line_distance) {
    auto [distance, right] = Split(Lstrip(line_distance), ' ');
    auto [_, stop] = Split(Lstrip(right), ' ');
    return {Lstrip(stop), std::stoi(distance.data())};
}

}
// end namespace detail

void InputReader::LoadStop(std::string_view line, QueriesDistances& queries_distances) {
    using namespace detail;
    auto [left, coords] = Split(line, ':');
    auto [_, name] = Split(left, ' ');
    auto [lat, tail] = Split(coords, ',');
    auto [lng, distance] = Split(tail, ',');
    catalogue_.AddStop(Lstrip(name), std::stod(lat.data()), std::stod(lng.data()));
    if(!distance.empty()) {
        queries_distances.push_back({std::string(Lstrip(name)), std::string(Lstrip(distance))});
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
                catalogue_.AddDistance(stop1, stop2, distance);
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
            auto stop_name = catalogue_.FindStop(Lstrip(stop));
            if(stop_name != nullptr) {
                bus_stops.push_back(const_cast<Stop*>(stop_name));
            }
            auto [left, right] = Split(right_src, delimeter);
            stop = left;
            right_src = right;
        }
        auto stop_name = catalogue_.FindStop(Lstrip(stop));
        if(stop_name != nullptr) {
            bus_stops.push_back(const_cast<Stop*>(stop_name));
        }

        if(delimeter == '-') {
            auto rit = bus_stops.end();
            while(--rit > bus_stops.begin()) {
                bus_stops.push_back(*(rit - 1));
            }
        }

        catalogue_.AddBus(Lstrip(bus_name), std::move(bus_stops));
    }
}

void InputReader::LoadQueries(std::deque<std::string>& queries) {
    StatReader stat_reader(catalogue_);
    for(auto& line : queries) {
        auto [query, name] = detail::Split(line, ' ');
        name = detail::Lstrip(name);
        if(detail::Lstrip(query) == "Bus") {
            std::cout << stat_reader.FormatBusInfo(name) << std::endl;
        } else {
            std::cout << stat_reader.FormatStopInfo(name) << std::endl;
        }
    }
}

void InputReader::Load(std::istream& input) {
    bool line_first = true, qline_first = true;
    int line_count = 0, qline_count = 0;
    std::deque<std::string> queries_buses, queries;
    QueriesDistances queries_distances;
    for (std::string gline; getline(input, gline);) {
        if(gline.empty()) {
            break;
        }
        if(line_first) {
            line_count = stoi(gline);
            line_first = false;
        } else {
            if(line_count-- > 0){
                if(gline[0] == 'S') {
                    LoadStop(gline, queries_distances);
                } else {
                    queries_buses.push_back(gline);
                }
            } else
            if(qline_first) {
                qline_count = stoi(gline);
                qline_first = false;
            } else {
                if(qline_count-- > 0){
                    queries.push_back(gline);
                }
            }
        }
    }
    LoadDistance(queries_distances);
    LoadBuses(queries_buses);
    LoadQueries(queries);
    //catalogue_.PrintTest();
}

}
