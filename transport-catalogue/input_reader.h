#pragma once

#include <iostream>
#include <string>
#include <deque>

#include "transport_catalogue.h"

namespace transport {

using QueriesDistances = std::deque<std::pair<std::string, std::string>>;

namespace detail {

std::pair<std::string_view, std::string_view> Split(std::string_view line, char by);

std::string_view SpaceTrim(std::string_view line);

std::pair<std::string_view, int> ParseDistance(std::string_view line_distance);

}

class InputReader {

public:
    InputReader(TransportCatalogue& catalogue) : catalogue_(catalogue) {};

    void LoadStop(std::string_view line, QueriesDistances& queries_distances);

    void LoadDistance(QueriesDistances& queries_distances);

    void LoadBuses(std::deque<std::string>& queries_buses);

    void Load(std::istream& input);

private:
    TransportCatalogue& catalogue_;
};

}
