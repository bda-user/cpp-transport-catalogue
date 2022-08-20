#include "serialization.h"

bool transport::Serial::SaveCatalogue(TransportCatalogue& catalogue_,
                                      transport::serial::Catalogue& catalogue) {

    for(const auto& stop_ : catalogue_.stops_) {
        transport::serial::Stop stop;
        stop.set_name(stop_.name);
        stop.set_lat(stop_.coordinates.lat);
        stop.set_lng(stop_.coordinates.lng);
        stop.set_id(stop_.id);
        *catalogue.add_stops() = std::move(stop);
    }

    for(const auto& [key, val] : catalogue_.distances_) {
        transport::serial::Distance dist;
        dist.set_from(key.from_ptr->id);
        dist.set_to(key.to_ptr->id);
        dist.set_val(val);
        *catalogue.add_distances() = std::move(dist);
    }

    for(const auto& bus_ : catalogue_.buses_) {
        transport::serial::Bus bus;
        bus.set_name(bus_.name);
        bus.set_id(bus_.id);
        bus_.last_stop == nullptr ?
                    bus.set_last_stop(-1) :
                    bus.set_last_stop(bus_.last_stop->id);
        for(const auto& stop : bus_.stops) bus.add_stops(stop->id);
        *catalogue.add_buses() = std::move(bus);
    }

    for(const auto& [stop_, buses_] : catalogue_.stops_buses_indx_) {
        transport::serial::StopBuses buses;
        buses.set_id(stop_->id);
        for(const auto& bus_ : buses_) buses.add_buses(bus_->id);
        *catalogue.add_stop_buses() = std::move(buses);
    }

    return true;
}

bool transport::Serial::SaveRenderSettings(
        renderer::RenderSettings& render_settings_,
        transport::serial::RenderSettings& render_settings) {

    render_settings.set_width(render_settings_.width);
    render_settings.set_height(render_settings_.height);
    render_settings.set_padding(render_settings_.padding);
    render_settings.set_stroke_width(render_settings_.stroke_width);
    render_settings.set_stop_radius(render_settings_.stop_radius);
    render_settings.set_bus_label_font_size(render_settings_.bus_label_font_size);
    transport::serial::Point point;
    point.set_x(render_settings_.bus_label_offset.x);
    point.set_y(render_settings_.bus_label_offset.y);
    *render_settings.mutable_bus_label_offset() = std::move(point);
    render_settings.set_stop_label_font_size(render_settings_.stop_label_font_size);
    transport::serial::Point point2;
    point2.set_x(render_settings_.stop_label_offset.x);
    point2.set_y(render_settings_.stop_label_offset.y);
    *render_settings.mutable_stop_label_offset() = std::move(point2);
    render_settings.set_underlayer_color(render_settings_.underlayer_color);
    render_settings.set_underlayer_width(render_settings_.underlayer_width);
    for(const auto& color_: render_settings_.stroke_color)
        render_settings.add_stroke_color(color_);
    render_settings.set_fill_color(render_settings_.fill_color);
    render_settings.set_stroke_line_join(
                static_cast<int>(render_settings_.stroke_line_join));
    render_settings.set_stroke_line_cap(
                static_cast<int>(render_settings_.stroke_line_cap));

    return true;
}

bool transport::Serial::SaveRouter(TransportRouter& router_,
                                   transport::serial::Router& router) {

    transport::serial::RouterSettings router_settings;
    router_settings.set_wait(router_.settings_.wait);
    router_settings.set_velocity(router_.settings_.velocity);
    *router.mutable_router_settings() = std::move(router_settings);

    for(const auto& [name_, number_] : router_.stops_) {
        transport::serial::RouterStop router_stop;
        router_stop.set_name(name_);
        router_stop.set_number(number_);
        *router.add_router_stops() = std::move(router_stop);
    }

    for(const auto& edge_ : router_.edges_) {
        transport::serial::RouterEdgeIdx edge;
        edge.set_bus_id(edge_.bus->id);
        edge.set_from_id(edge_.from->id);
        edge.set_to_id(edge_.to->id);
        edge.set_time(edge_.time);
        edge.set_span(edge_.span);
        *router.add_router_edge_idx() = std::move(edge);
    }

    return true;
}

bool transport::Serial::SaveGraph(TransportRouter& router_,
                                  transport::serial::Graph& graph) {

    for(const auto& edge_ : router_.graph_->edges_) {
        transport::serial::GraphEdge edge;
        edge.set_from(edge_.from);
        edge.set_to(edge_.to);
        edge.set_weight(edge_.weight);
        *graph.add_grath_edges() = std::move(edge);
    }

    for(const auto& list_ : router_.graph_->incidence_lists_) {
        transport::serial::GraphIncidenceList list;
        for(const auto& edge_id_ : list_) list.add_edge_ids(edge_id_);
        *graph.add_grath_incidence_lists() = std::move(list);
    }

    for(const auto& routes_internal_data_ : router_.router_->routes_internal_data_) {
        transport::serial::RoutesInternalData routes_internal_data;
        for(const auto& route_internal_data_ : routes_internal_data_) {
            transport::serial::RouteInternalData route_internal_data;
            if(route_internal_data_ == std::nullopt) {
                route_internal_data.set_weight(-1.0);
            } else {
                route_internal_data.set_weight(route_internal_data_->weight);
                route_internal_data.set_prev_edge(
                            route_internal_data_->prev_edge == std::nullopt ?
                            -1 : route_internal_data_->prev_edge.value());
            }
            *routes_internal_data.add_route_internal_data() =
                        std::move(route_internal_data);
        }
        *graph.add_routes_internal_data() = std::move(routes_internal_data);
    }

    return true;
}

bool transport::Serial::SaveBase(std::string fname,
                                 TransportCatalogue& catalogue_,
                                 renderer::RenderSettings& render_settings_,
                                 TransportRouter& router_) {

    transport::serial::TransportCatalogue base;

    // CATALOGUE
    transport::serial::Catalogue catalogue;
    SaveCatalogue(catalogue_, catalogue);
    *base.mutable_catalogue() = std::move(catalogue);

    // RENDER_SETTINGS
    transport::serial::RenderSettings render_settings;
    SaveRenderSettings(render_settings_, render_settings);
    *base.mutable_render_settings() = std::move(render_settings);

    // ROUTER
    transport::serial::Router router;
    SaveRouter(router_, router);
    *base.mutable_router() = std::move(router);

    // GRAPH
    transport::serial::Graph graph;
    SaveGraph(router_, graph);
    *base.mutable_graph() = std::move(graph);

    std::ofstream out_file(fname, std::ios::binary);
    base.SerializeToOstream(&out_file);

    return true;
}

bool transport::Serial::LoadCatalogue(
        transport::serial::TransportCatalogue& base,
        TransportCatalogue& catalogue,
        std::vector<transport::Stop*>& stops,
        std::vector<transport::Bus*>& buses) {

    for(const auto& stop : base.mutable_catalogue()->stops()) {
        stops[stop.id()] = catalogue.AddStop(stop.name(), {stop.lat(), stop.lng()});
    }

    for(const auto& dist : base.mutable_catalogue()->distances()) {
        catalogue.SetDistance(stops[dist.from()], stops[dist.to()], dist.val());
    }

    for(auto& bus : base.mutable_catalogue()->buses()) {
        std::deque<Stop*> bus_stops;
        for(auto stop_id : bus.stops()) bus_stops.push_back(stops[stop_id]);
        buses[bus.id()] = catalogue.AddBus(bus.name(), std::move(bus_stops),
                                           bus.last_stop() == -1 ?
                                           nullptr : stops[bus.last_stop()]);
    }

    for(const auto& stop : base.mutable_catalogue()->stop_buses()) {
        std::unordered_set<Bus*> stop_buses;
        for(auto bus_id : stop.buses()) stop_buses.insert(buses[bus_id]);
        catalogue.stops_buses_indx_.insert({stops[stop.id()], std::move(stop_buses)});
    }

    return true;
}

bool transport::Serial::LoadRenderSettings(
        transport::serial::TransportCatalogue& base,
        renderer::RenderSettings& render_settings_) {

    const auto& render_settings = base.render_settings();
    render_settings_.width = render_settings.width();
    render_settings_.height = render_settings.height();
    render_settings_.padding = render_settings.padding();
    render_settings_.stroke_width = render_settings.stroke_width();
    render_settings_.stop_radius = render_settings.stop_radius();
    render_settings_.bus_label_font_size = render_settings.bus_label_font_size();
    render_settings_.bus_label_offset = {render_settings.bus_label_offset().x(),
                                         render_settings.bus_label_offset().y()};
    render_settings_.stop_label_font_size = render_settings.stop_label_font_size();
    render_settings_.stop_label_offset = {render_settings.stop_label_offset().x(),
                                          render_settings.stop_label_offset().y()};
    render_settings_.underlayer_color = render_settings.underlayer_color();
    render_settings_.underlayer_width = render_settings.underlayer_width();
    render_settings_.stroke_color.resize(0);
    for(auto& color : render_settings.stroke_color())
        render_settings_.stroke_color.push_back(color);
    render_settings_.fill_color = render_settings.fill_color();
    render_settings_.stroke_line_join = static_cast<svg::StrokeLineJoin>(
                render_settings.stroke_line_join());
    render_settings_.stroke_line_cap = static_cast<svg::StrokeLineCap>(
                render_settings.stroke_line_cap());

    return true;
}

bool transport::Serial::LoadRouter(transport::serial::TransportCatalogue& base,
                                   TransportRouter& router_,
                                   std::vector<transport::Stop*>& stops,
                                   std::vector<transport::Bus*>& buses) {

    const auto& router_settings = base.router().router_settings();
    router_.settings_.wait = router_settings.wait();
    router_.settings_.velocity = router_settings.velocity();

    router_.stops_.clear();
    for(const auto& stop : base.router().router_stops()) {
        router_.stops_.insert({stop.name(), stop.number()});
    }

    router_.edges_.clear();
    for(const auto& edge : base.router().router_edge_idx()) {
        router_.edges_.push_back({buses[edge.bus_id()], stops[edge.from_id()],
                                  stops[edge.to_id()], edge.time(), edge.span()});
    }

    return true;
}

bool transport::Serial::LoadGraph(transport::serial::TransportCatalogue& base,
                                  TransportRouter& router_) {

    router_.graph_->edges_.clear();
    for(const auto& edge : base.graph().grath_edges()) {
        router_.graph_->edges_.push_back({edge.from(), edge.to(), edge.weight()});
    }

    router_.graph_->incidence_lists_.clear();
    for(const auto& list : base.graph().grath_incidence_lists()) {
        std::vector<graph::EdgeId> list_;
        for(const auto& edge_id : list.edge_ids()) list_.push_back(edge_id);
        router_.graph_->incidence_lists_.push_back(std::move(list_));
    }

    router_.router_->routes_internal_data_.clear();
    for(const auto& routes_internal_data : base.graph().routes_internal_data()) {

        std::vector<std::optional<graph::Router<double>::RouteInternalData>> routes_internal_data_;
        for(const auto& route_internal_data : routes_internal_data.route_internal_data()) {

            std::optional<graph::Router<double>::RouteInternalData> route_internal_data_;
            if(route_internal_data.weight() != -1.0) {
                std::optional<graph::EdgeId> prev_edge;
                if(route_internal_data.prev_edge() != -1) {
                    prev_edge = route_internal_data.prev_edge();
                }
                route_internal_data_ = {route_internal_data.weight(), prev_edge};
            }
            routes_internal_data_.push_back(std::move(route_internal_data_));
        }
        router_.router_->routes_internal_data_.push_back(std::move(routes_internal_data_));
    }

    return true;
}

bool transport::Serial::LoadBase(std::string fname,
                                 TransportCatalogue& catalogue_,
                                 renderer::RenderSettings& render_settings_,
                                 TransportRouter& router_) {

    std::ifstream in_file(fname, std::ios::binary);

    transport::serial::TransportCatalogue base;

    if (!base.ParseFromIstream(&in_file)) {
        return false;
    }

    std::vector<transport::Stop*> stops(base.mutable_catalogue()->stops_size());
    std::vector<transport::Bus*> buses(base.mutable_catalogue()->buses_size());

    // CATALOGUE
    TransportCatalogue catalogue;
    LoadCatalogue(base, catalogue, stops, buses);
    catalogue_ = std::move(catalogue);

    // RENDER_SETTINGS
    renderer::RenderSettings render_settings;
    LoadRenderSettings(base, render_settings);
    render_settings_ = std::move(render_settings);

    // ROUTER
    LoadRouter(base, router_, stops, buses);

    // GRAPH
    router_.graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(2 * catalogue_.GetStops().size());
    router_.router_ = std::make_unique<graph::Router<double>>(*router_.graph_);

    LoadGraph(base, router_);

    return true;
}
