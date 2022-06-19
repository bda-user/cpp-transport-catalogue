#include "map_renderer.h"

using namespace std::literals;

namespace renderer {

void MapRenderer::AddRoute(svg::Document& doc, const Route& stops,
                                     const renderer::RenderSettings& rs,
                                     std::string color) const {
    svg::Polyline route;
    for(const auto& stop : stops) {
        route.AddPoint(stop);
    }

    doc.Add(std::move(route.SetFillColor(rs.fill_color)
                      .SetStrokeColor(color)
                      .SetStrokeWidth(rs.stroke_width)
                      .SetStrokeLineJoin(rs.stroke_line_join)
                      .SetStrokeLineCap(rs.stroke_line_cap)));
}

void MapRenderer::AddRouteLabel(svg::Document& doc, const RouteLabel& bus_label,
                              const renderer::RenderSettings& rs, std::string color) const {
    using namespace svg;
    using namespace std;

    const Text label = Text()
            .SetFontFamily("Verdana"s)
            .SetFontWeight("bold"s)
            .SetOffset(rs.bus_label_offset)
            .SetFontSize(rs.bus_label_font_size);

    const Text underlayer = Text{label}
            .SetStrokeLineJoin(StrokeLineJoin::ROUND)
            .SetStrokeLineCap(StrokeLineCap::ROUND)
            .SetStrokeColor(rs.underlayer_color)
            .SetFillColor(rs.underlayer_color)
            .SetStrokeWidth(rs.underlayer_width);

    doc.Add(Text{underlayer}
            .SetPosition(bus_label.first)
            .SetData(bus_label.name));

    doc.Add(Text{label}
            .SetPosition(bus_label.first)
            .SetData(bus_label.name)
            .SetFillColor(color));

    if(bus_label.second.x == 0.0 && bus_label.second.y == 0.0) return;

    doc.Add(Text{underlayer}
            .SetPosition(bus_label.second)
            .SetData(bus_label.name));

    doc.Add(Text{label}
            .SetPosition(bus_label.second)
            .SetData(bus_label.name)
            .SetFillColor(color));

}

void MapRenderer::AddStops(svg::Document& doc, const std::vector<RouteStop>& stops, const renderer::RenderSettings& rs) const {
    using namespace svg;
    using namespace std;

    for(const auto& stop : stops) {
        doc.Add(Circle()
                .SetFillColor("white"s)
                .SetRadius(rs.stop_radius)
                .SetCenter(stop.coords));
    }

}

void MapRenderer::AddStopLabel(svg::Document& doc, const RouteStop& stop, const RenderSettings& rs) const {
    using namespace svg;
    using namespace std;

    const Text label = Text()
            .SetFontFamily("Verdana"s)
            .SetOffset(rs.stop_label_offset)
            .SetFontSize(rs.stop_label_font_size)
            .SetPosition(stop.coords)
            .SetData(stop.name);

    doc.Add(Text{label}
            .SetStrokeLineJoin(StrokeLineJoin::ROUND)
            .SetStrokeLineCap(StrokeLineCap::ROUND)
            .SetStrokeColor(rs.underlayer_color)
            .SetFillColor(rs.underlayer_color)
            .SetStrokeWidth(rs.underlayer_width));

    doc.Add(Text{label}
            .SetFillColor("black"s));

}

svg::Document MapRenderer::RenderMap(const renderer::RenderLayers& layers,
                                     const renderer::RenderSettings& rs) const {
    std::vector<std::unique_ptr<svg::Drawable>> picture;
    svg::Document doc;

    // Routs
    int j = 0;
    for(const auto& stops : layers.routs) {
        AddRoute(doc, stops, rs, rs.stroke_color[j % rs.stroke_color.size()]);
        ++j;
    }

    // Labels
    j = 0;
    for(const auto& bus_label : layers.labels) {
        AddRouteLabel(doc, bus_label, rs, rs.stroke_color[j % rs.stroke_color.size()]);
        ++j;
    }

    // Stops
    AddStops(doc, layers.stops, rs);

    // StopsLabel
    for(const auto& stop : layers.stops) {
        AddStopLabel(doc, stop, rs);
    }

    return doc;
}


} // namespace renderer
