#pragma once

#include <optional>

#include "geo.h"
#include "svg.h"
#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

namespace transport {

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer)
        : db_(db), renderer_(renderer) {};

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<transport::BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::unordered_set<transport::BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    // & fill Routs
    SphereProjector MakeSphereProjector(const renderer::RenderSettings& render_settings);

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap(const renderer::RenderSettings& render_rettings);

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transport::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    std::vector<Bus*> buses_; // Buses with Stops & sorted by BusName
    //std::vector<Stop*> stops_; // Stops sorted by StopName
    std::unordered_set<Stop*> stops_;
    renderer::RenderLayers layers_;
};

} //namespace transport
