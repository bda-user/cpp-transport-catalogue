#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"

namespace transport {

class JsonReader {

public:
    JsonReader(TransportCatalogue& catalogue, json::Node node,
               RequestHandler& request_handler)
        : catalogue_(catalogue), root_node_(std::move(node)), request_handler_(request_handler) {};

    void FillDataBaseStops(const json::Array& base_reqs);

    void FillDataBaseBuses(const json::Array& base_reqs);

    void FillDataBase();

    json::Dict ExecQueryStop(std:: string stop_name, int req_id);

    json::Dict ExecQueryBus(std:: string bus_name, int req_id);

    void ExecQueries();

    std::string FormatColor(const json::Node& color) const;

    void FillColorPalette(const json::Node& color_palette, std::vector<std::string>& vec_color);

    std::string RenderMap();

    bool SetRouterSettings();

    json::Node GetNodeValue(TransportRouter::ItemValue value);

    json::Dict ExecQueryRoute(std:: string from, std:: string to, int req_id);

private:
    TransportCatalogue& catalogue_;
    json::Node root_node_;
    RequestHandler& request_handler_;
    renderer::RenderSettings render_rettings_;
};

} //namespace transport
