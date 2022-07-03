#include "transport_router.h"

#include <iostream>

namespace transport {

void TransportRouter::SetSettings(Settings settings) {
    wait_ = settings.wait;
    velocity_ = settings.velocity;

    size_t i = 0;
    const auto& stops = catalog_.GetStops();

    // add stop shadow f.e. "Universam" -> "Universam_#_"
    for(Stop stop : stops) {
        std::string stop_suff = stop.name + STOP_SUFFIX;
        stops_.insert({std::move(stop.name), i++});
        stops_.insert({std::move(stop_suff), i++});
    }

    graph_ = new graph::DirectedWeightedGraph<double>(2 * catalog_.GetStops().size());
    FillGraph();
    router_ = new graph::Router<double>(*graph_);
}

void TransportRouter::AddEdges(EdgeIdx edge_idx, std::vector<double>& span_time) {

    size_t from = stops_.at(edge_idx.from->name);
    size_t from_suff = stops_.at(edge_idx.from->name + STOP_SUFFIX);

    // 1) add edge for stop -> shadow
    // f.e. (enter)"Universam" (wait bus)-> (leave)"Universam_#_"
    // A - B - C here A - A_#_
    graph_->AddEdge({from, from_suff, wait_});
    edges_.push_back({edge_idx.bus, edge_idx.from, edge_idx.from, wait_, 0});

    // 2) add edge (span etc.) for each bus stops pair
    // A - B - C here A_#_ - B
    graph_->AddEdge({from_suff, stops_.at(edge_idx.to->name), edge_idx.time});
    edges_.push_back(edge_idx);

    // 3) additional edges for bus: from {begin() ... current - 2}, to{current}
    // A - B - C here A_#_ - C , two span: (A - B) + (B - C)
    auto it = edge_idx.bus->stops.begin();
    for(size_t i = 0; i < span_time.size() - 1; ++i) {
        size_t span = span_time.size() - i;
        if(span < 2) continue;

        auto stop = *it++;

        size_t from_suff = stops_.at(stop->name + STOP_SUFFIX);
        graph_->AddEdge({from_suff, stops_.at(edge_idx.to->name), span_time[i]});
        edges_.push_back({edge_idx.bus, stop, edge_idx.to, span_time[i], span});
    }
}

void TransportRouter::FillGraph() {
    for(const auto& bus : catalog_.GetBuses()) {
        Stop *from, *to;
        bool first_step = true;

        std::vector<double> span_time;
        span_time.reserve(bus.stops.size());

        for(auto& stop : bus.stops) {
            to = stop;
            if(first_step) {
                first_step = false;
                from = to;
                continue;
            }

            double dist = catalog_.GetDistance(from, to);
            double time = 60.0 * dist / 1000 / velocity_;

            /*
             * for AddEdges() p. 3)
             * A B C
             * +
             * + +
             * + + +
            */
            span_time.push_back(0.0);
            for(auto& span_time : span_time) {
                span_time += time;
            }

            AddEdges({const_cast<Bus*>(&bus), from, to, time}, span_time);
            from = to;
        }
    }
}

std::optional<TransportRouter::Route> TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {
    auto route =  router_->BuildRoute(stops_.at(std::string(from)),
                                      stops_.at(std::string(to)));
    if(route == std::nullopt) {
        return std::nullopt;
    }

    Route answer;
    answer.total_time = route->weight;

    for(const auto& stop : route->edges) {
        auto edge_idx = edges_.at(stop);

        if(edge_idx.span == 0) {
            answer.items.push_back({
                                    {"stop_name"s, edge_idx.from->name},
                                    {"time"s, edge_idx.time},
                                    {"type"s, "Wait"s}
                                });
        } else {
            answer.items.push_back({
                                    {"bus"s, edge_idx.bus->name},
                                    {"span_count"s, (int)edge_idx.span},
                                    {"time"s, edge_idx.time},
                                    {"type"s, "Bus"s}
                                });
        }
    }
    return answer;
}

} // namespace transport
