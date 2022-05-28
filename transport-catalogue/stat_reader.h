#pragma once

#include <iostream>
#include <string>
#include <deque>

#include "transport_catalogue.h"

namespace transport {

class StatReader {

public:
    StatReader(TransportCatalogue catalogue) : catalogue_(catalogue) {};

    std::string FormatStopInfo(std::string_view);

    std::string FormatBusInfo(std::string_view);

private:
    TransportCatalogue catalogue_;
};

}
