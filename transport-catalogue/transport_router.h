#pragma once

#include "geo.h"
#include "router.h"
#include "transport_catalogue.h"
#include "serialization.h"

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <variant>
#include <optional>
#include <memory>

using namespace std::literals;

namespace transport {

class TransportCatalogue;

class TransportRouter {

    friend class Serial;

const std::string STOP_SUFFIX = "_#_"s;

using RouteInfo = std::optional<graph::Router<double>::RouteInfo>;

public:

    struct Settings {
        double wait = 0.0;
        double velocity = 0.0;
    };

    using ItemValue = std::variant<std::string, int, double>;

    struct Route {
        std::vector<std::unordered_map<std::string, ItemValue>> items;
        double total_time;
    };

    TransportRouter(TransportCatalogue& catalog) : catalog_(catalog) {}

    void Init(Settings);

    std::optional<Route> BuildRoute(std::string_view from, std::string_view to) const;

private:

    struct EdgeIdx {
        Bus* bus;
        Stop* from;
        Stop* to;
        double time;
        size_t span = 1;
    };

    void FillGraph();

    void AddEdges(EdgeIdx, std::vector<double>&);

    const TransportCatalogue& catalog_;
    Settings settings_{0.0, 0.0};
    std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
    std::unique_ptr<graph::Router<double>> router_;
    std::unordered_map<std::string, size_t> stops_;
    std::vector<EdgeIdx> edges_;
};

}
