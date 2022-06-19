#include <vector>
#include <algorithm>

#include "json_reader.h"

namespace transport {

void JsonReader::FillDataBaseStops(const json::Array& base_reqs) {
    for(auto& reqs : base_reqs) {
        if(reqs.AsMap().at("type"s) != "Stop"s) continue;
        auto& req = reqs.AsMap();
        catalogue_.AddStop(req.at("name"s).AsString(),
                           {req.at("latitude"s).AsDouble(),
                            req.at("longitude"s).AsDouble()});
    }

    for(auto& req_node : base_reqs) {
        auto& req = req_node.AsMap();
        if(req.at("type"s) != "Stop"s) continue;

        const auto& distances_it = req.find("road_distances"s);
        if(distances_it == req.end()) continue;

        auto from = catalogue_.FindStop(req.at("name"s).AsString());
        if(from == nullptr) {
            continue;
        }
        for(auto& [stop, distance] : distances_it->second.AsMap()){
            auto to = catalogue_.FindStop(stop);
            if(to != nullptr) {
                catalogue_.SetDistance(from, to, distance.AsInt());
            }
        }
    }
}

void JsonReader::FillDataBaseBuses(const json::Array& base_reqs) {
    for(auto& reqs : base_reqs) {
        if(reqs.AsMap().at("type"s) != "Bus"s) continue;
        auto& req = reqs.AsMap();
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
    const auto& base_reqs_it = root_node_.AsMap().find("base_requests"s);
    if(base_reqs_it == root_node_.AsMap().end()) return;
    auto& base_reqs = base_reqs_it->second.AsArray();

    FillDataBaseStops(base_reqs);

    FillDataBaseBuses(base_reqs);

    return;
}

void JsonReader::ExecQueryStop(std:: string stop_name, json::Dict& answer){
    using namespace std;
    using namespace json;

    if(catalogue_.FindStop(stop_name) == nullptr) {
        answer.insert(pair<string, string>("error_message"s, "not found"s));
    } else {
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
        answer.insert(pair<string, Array>("buses"s, arr_buses));
    }
}

void JsonReader::ExecQueryBus(std:: string bus_name, json::Dict& answer) {
    using namespace std;

    auto bus_stat = request_handler_.GetBusStat(bus_name);
    if(bus_stat) {
        answer.insert(pair<string, double>("curvature"s, bus_stat->curvature));
        answer.insert(pair<string, int>("route_length"s, bus_stat->route_length));
        answer.insert(pair<string, int>("stop_count"s, bus_stat->stop_count));
        answer.insert(pair<string, int>("unique_stop_count"s, bus_stat->unique_stop_count));
    } else {
        answer.insert(pair<string, string>("error_message"s, "not found"s));
    }
}

void JsonReader::ExecQueries(){
    using namespace std;
    using namespace json;

    const auto& stat_reqs_it = root_node_.AsMap().find("stat_requests"s);
    if(stat_reqs_it == root_node_.AsMap().end()) return;
    auto& stat_reqs = stat_reqs_it->second.AsArray();

    Array answers{};
    for(auto& reqs : stat_reqs) {
        auto& req = reqs.AsMap();
        Dict answer;
        answer.insert(pair<string, int>("request_id"s, req.at("id"s).AsInt()));

        if(req.at("type"s) == "Stop"s) {
            ExecQueryStop(req.at("name"s).AsString(), answer);
        } else
        if(req.at("type"s) == "Bus"s) {
            ExecQueryBus(req.at("name"s).AsString(), answer);
        } else
        if(req.at("type"s) == "Map"s) {
            answer.insert(pair<string, string>("map"s, RenderMap()));
        }
        answers.push_back(answer);
    }

    Node node_answers{answers};
    std::stringstream strm;
    PrintContext ctx{strm};
    Print(json::Document{node_answers}, ctx);
    std::cout << ctx.out.str();
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

    const auto& render_sets_it = root_node_.AsMap().find("render_settings"s);
    if(render_sets_it == root_node_.AsMap().end()) return ""s;
    const auto& set = render_sets_it->second.AsMap();

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

} //namespace transport
