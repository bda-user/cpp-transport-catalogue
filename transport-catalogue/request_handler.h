#pragma once

#include "geo.h"
#include "svg.h"
#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <optional>

namespace transport {

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue& db,
                   const renderer::MapRenderer& renderer,
                   transport::TransportRouter& router)

        : db_(db), renderer_(renderer), router_(router) {};

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<transport::BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::unordered_set<transport::BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    // & fill Routs
    SphereProjector MakeSphereProjector(const renderer::RenderSettings& render_settings);

    svg::Document RenderMap(const renderer::RenderSettings& render_rettings);

    void TuneRouter(const TransportRouter::Settings settings);

    std::optional<TransportRouter::Route> BuildRoute(std::string_view from, std::string_view to) const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transport::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    std::vector<Bus*> buses_; // Buses with Stops & sorted by BusName

    std::unordered_set<Stop*> stops_;
    renderer::RenderLayers layers_;
    transport::TransportRouter& router_;
};

} //namespace transport
