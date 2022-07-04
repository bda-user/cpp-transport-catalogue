#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

#include "json_reader.h"
#include "json_builder.h"

namespace transport {

void JsonReader::FillDataBaseStops(const json::Array& base_reqs) {
    for(auto& reqs : base_reqs) {
        if(reqs.AsDict().at("type"s) != "Stop"s) continue;
        auto& req = reqs.AsDict();
        catalogue_.AddStop(req.at("name"s).AsString(),
                           {req.at("latitude"s).AsDouble(),
                            req.at("longitude"s).AsDouble()});
    }

    for(auto& req_node : base_reqs) {
        auto& req = req_node.AsDict();
        if(req.at("type"s) != "Stop"s) continue;

        const auto& distances_it = req.find("road_distances"s);
        if(distances_it == req.end()) continue;

        auto from = catalogue_.FindStop(req.at("name"s).AsString());
        if(from == nullptr) {
            continue;
        }
        for(auto& [stop, distance] : distances_it->second.AsDict()){
            auto to = catalogue_.FindStop(stop);
            if(to != nullptr) {
                catalogue_.SetDistance(from, to, distance.AsInt());
            }
        }
    }
}

void JsonReader::FillDataBaseBuses(const json::Array& base_reqs) {
    for(auto& reqs : base_reqs) {
        if(reqs.AsDict().at("type"s) != "Bus"s) continue;
        auto& req = reqs.AsDict();
        std::deque<Stop*> bus_stops;

        // no stops
        if(req.find("stops"s) != req.end()) {
            for(auto& stop : req.at("stops"s).AsArray()) {
                auto stop_name = catalogue_.FindStop(stop.AsString());
                if(stop_name != nullptr) {
                    bus_stops.push_back(const_cast<Stop*>(stop_name));
                }
            }
        }

        Stop* last_stop{nullptr};
        if(req.at("is_roundtrip"s).AsBool() == false) {
            auto rit = bus_stops.end();
            last_stop = *(rit - 1);
            while(--rit > bus_stops.begin()) {
                bus_stops.push_back(*(rit - 1));
            }
        }

        catalogue_.AddBus(req.at("name"s).AsString(), std::move(bus_stops), last_stop);
    }
}

void JsonReader::FillDataBase() {
    const auto& base_reqs_it = root_node_.AsDict().find("base_requests"s);
    if(base_reqs_it == root_node_.AsDict().end()) return;
    auto& base_reqs = base_reqs_it->second.AsArray();

    FillDataBaseStops(base_reqs);

    FillDataBaseBuses(base_reqs);

    return;
}

json::Dict JsonReader::ExecQueryStop(std:: string stop_name, int req_id){
    using namespace std;
    using namespace json;

    if(catalogue_.FindStop(stop_name) == nullptr){
        return json::Builder{}.StartDict()
                            .Key("request_id"s).Value(req_id)
                            .Key("error_message"s).Value("not found"s)
                            .EndDict().Build().AsDict();
    }

    vector<string_view> vec_buses;
    const unordered_set<transport::BusPtr>* buses =
            request_handler_.GetBusesByStop(stop_name);
    if(buses != nullptr) {
        for(const auto bus : *buses) {
            vec_buses.push_back(bus->name);
        }
    }
    std::sort(vec_buses.begin(), vec_buses.end());
    Array arr_buses{};
    for(auto bus : vec_buses) {
        arr_buses.push_back(static_cast<string>(bus));
    }

    return json::Builder{}.StartDict()
                        .Key("request_id"s).Value(req_id)
                        .Key("buses"s).Value(arr_buses)
                        .EndDict().Build().AsDict();
}

json::Dict JsonReader::ExecQueryBus(std:: string bus_name, int req_id) {
    using namespace std;

    auto bus_stat = request_handler_.GetBusStat(bus_name);
    if(bus_stat) {
        return json::Builder{}.StartDict()
                            .Key("request_id"s).Value(req_id)
                            .Key("curvature"s).Value(bus_stat->curvature)
                            .Key("route_length"s).Value(bus_stat->route_length)
                            .Key("stop_count"s).Value(bus_stat->stop_count)
                            .Key("unique_stop_count"s).Value(bus_stat->unique_stop_count)
                            .EndDict().Build().AsDict();
    } else {
        return json::Builder{}.StartDict()
                            .Key("request_id"s).Value(req_id)
                            .Key("error_message"s).Value("not found"s)
                            .EndDict().Build().AsDict();
    }
}

void JsonReader::ExecQueries(){
    using namespace std;
    using namespace json;

    const auto& stat_reqs_it = root_node_.AsDict().find("stat_requests"s);
    if(stat_reqs_it == root_node_.AsDict().end()) return;
    auto& stat_reqs = stat_reqs_it->second.AsArray();

    const bool has_router_settings = SetRouterSettings();

    Array answers{};
    for(auto& reqs : stat_reqs) {

        string type = reqs.AsDict().at("type"s).AsString();
        int req_id = reqs.AsDict().at("id"s).AsInt();

        if(type == "Stop"s) {
            answers.push_back(ExecQueryStop(reqs.AsDict().at("name"s).AsString(), req_id));
        } else
        if(type == "Bus"s) {
            answers.push_back(ExecQueryBus(reqs.AsDict().at("name"s).AsString(), req_id));
        } else
        if(type == "Map"s) {
            answers.push_back(Builder{}.StartDict()
                              .Key("request_id"s).Value(req_id)
                              .Key("map"s).Value(RenderMap())
                              .EndDict().Build().AsDict());
        } else
        if(type == "Route"s && has_router_settings) {
            answers.push_back(ExecQueryRoute(reqs.AsDict().at("from"s).AsString(),
                                             reqs.AsDict().at("to"s).AsString(),
                                             req_id));
        }

    }

    Print(json::Document{Builder{}.Value(answers).Build()}, cout);
}

std::string JsonReader::FormatColor(const json::Node& color) const {

    if(color.IsString()) return color.AsString();

    std::string c = ""s;

    if(color.IsArray()) {
        bool first = true;
        int i = 0;
        bool rgba = false;
        for(const auto& clr : color.AsArray()) {
            if(first) {
                first = false;
            } else {
                c += ","s;
            }
            if(i < 3) {
                c += std::to_string(clr.AsInt());
            } else {
                std::ostringstream strs;
                strs << clr.AsDouble();
                c += strs.str();
                rgba = true;
            }
            ++i;
        }
        c = rgba ? "rgba("s + c : "rgb("s + c;
        c += ")"s;
    }
    return c;
}

void JsonReader::FillColorPalette(const json::Node& color_palette, std::vector<std::string>& vec_color) {
    for(const auto& color : color_palette.AsArray()) {
        std::string c = FormatColor(color);
        vec_color.push_back(move(c));
    }
}

std::string JsonReader::RenderMap() {
    using namespace std;
    using namespace json;

    const auto& render_sets_it = root_node_.AsDict().find("render_settings"s);
    if(render_sets_it == root_node_.AsDict().end()) return ""s;
    const auto& set = render_sets_it->second.AsDict();

    render_rettings_.width = set.at("width"s).AsDouble();
    render_rettings_.height = set.at("height"s).AsDouble();
    render_rettings_.padding = set.at("padding"s).AsDouble();
    render_rettings_.stroke_width = set.at("line_width"s).AsDouble();
    render_rettings_.stop_radius = set.at("stop_radius"s).AsDouble();
    render_rettings_.bus_label_font_size = set.at("bus_label_font_size"s).AsInt();

    const auto& blo = set.at("bus_label_offset"s).AsArray();
    render_rettings_.bus_label_offset = svg::Point{blo[0].AsDouble(), blo[1].AsDouble()};

    render_rettings_.stop_label_font_size = set.at("stop_label_font_size"s).AsInt();

    const auto& slo = set.at("stop_label_offset"s).AsArray();
    render_rettings_.stop_label_offset = svg::Point{slo[0].AsDouble(), slo[1].AsDouble()};

    render_rettings_.underlayer_color = FormatColor(set.at("underlayer_color"s));
    render_rettings_.underlayer_width = set.at("underlayer_width"s).AsDouble();

    vector<string> color_palette;
    FillColorPalette(set.at("color_palette"s), color_palette);

    render_rettings_.stroke_color = move(color_palette);

    std::ostringstream ss;
    request_handler_.RenderMap(render_rettings_).Render(ss);

    return ss.str();
}

bool JsonReader::SetRouterSettings() {
    const auto& router_sets_it = root_node_.AsDict().find("routing_settings"s);
    if(router_sets_it == root_node_.AsDict().end()) return false;

    const auto& set = router_sets_it->second.AsDict();

    TransportRouter::Settings settings;
    settings.velocity = set.at("bus_velocity"s).AsDouble();
    settings.wait = set.at("bus_wait_time"s).AsDouble();
    request_handler_.InitRouter(settings);
    return true;
}

json::Node JsonReader::GetNodeValue(TransportRouter::ItemValue value) {
    using namespace json;
    if(std::holds_alternative<double>(value))
        return Node(std::get<double>(value));
    else
    if(std::holds_alternative<int>(value))
        return Node(std::get<int>(value));
    else
        return Node(std::get<std::string>(value));
}

json::Dict JsonReader::ExecQueryRoute(std:: string from, std:: string to, int req_id){

    using namespace json;

    auto route = request_handler_.BuildRoute(from, to);
    if(route == std::nullopt) {
        return Builder{}.StartDict()
            .Key("request_id"s).Value(req_id)
            .Key("error_message"s).Value("not found"s)
        .EndDict().Build().AsDict();
    }

    Array items{};
    for(auto& item : route->items) {
        Dict dict{};
        for(auto& [key, value] : item) {
            Node node;
            dict.insert({key, GetNodeValue(value)});
        }
        items.push_back(std::move(dict));
    }

    return json::Builder{}
                        .StartDict()
                            .Key("items"s).Value(items)
                            .Key("request_id"s).Value(req_id)
                            .Key("total_time"s).Value(route->total_time)
                        .EndDict().Build().AsDict();
}

} //namespace transport
