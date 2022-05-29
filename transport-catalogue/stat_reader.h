#pragma once

#include <iostream>
#include <string>
#include <deque>

#include "transport_catalogue.h"

namespace transport {

class StatReader {

public:
    StatReader(TransportCatalogue& catalogue) : catalogue_(catalogue) {};

    void LoadQueries(std::istream& input);

    void ExecQueries(std::ostream& output);

    std::string FormatStopInfo(std::string_view);

    std::string FormatBusInfo(std::string_view);

private:
    std::deque<std::string> queries_;
    TransportCatalogue& catalogue_;
};

}
