#include <iostream>
#include <sstream>
#include "svg.h"

namespace svg {

using namespace std::literals;

struct OstreamColor {
    std::ostream& out;

    void operator()(std::monostate) const {
        out << "none"sv;
    }
    void operator()(const std::string color) const {
        out << color;
    }
    void operator()(const svg::Rgb color) const {
        out << "rgb("sv << unsigned(color.red) << ","sv << unsigned(color.green) << ","sv  << unsigned(color.blue) << ")"sv;
    }
    void operator()(const svg::Rgba color) const {
        out << "rgba("sv << unsigned(color.red) << ","sv << unsigned(color.green) << ","sv  << unsigned(color.blue) << ","sv  << color.opacity << ")"sv;
    }
};

std::ostream& operator<< (std::ostream& out, const svg::Color color) {
    std::ostringstream strm;
    std::visit(OstreamColor{strm}, color);
    out << strm.str();
    return out;
}

std::ostream& operator<< (std::ostream& out, const svg::StrokeLineCap prop) {
    switch (prop) {
    case StrokeLineCap::BUTT:
        out << "butt"sv;
        break;
    case StrokeLineCap::ROUND:
        out << "round"sv;
        break;
    case StrokeLineCap::SQUARE:
        out << "square"sv;
        break;
    default:
        break;
    }
    return out;
}

std::ostream& operator<< (std::ostream& out, const svg::StrokeLineJoin prop) {
    switch (prop) {
    case StrokeLineJoin::ARCS:
        out << "arcs"sv;
        break;
    case StrokeLineJoin::BEVEL:
        out << "bevel"sv;
        break;
    case StrokeLineJoin::MITER:
        out << "miter"sv;
        break;
    case StrokeLineJoin::MITER_CLIP:
        out << "miter-clip"sv;
        break;
    case StrokeLineJoin::ROUND:
        out << "round"sv;
        break;
    default:
        break;
    }
    return out;
}

// ---------- Object ------------------

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point)  {
    points_.emplace_back(std::move(point));
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool first = true;
    for(auto& point : points_) {
        if(first)
            first = false;
        else
            out << " "sv;
        out << point.x << ","sv << point.y;
    }
    out << "\" "sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
    pos_ = std::move(pos);
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = std::move(offset);
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text "sv;
    RenderAttrs(out);
    out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << size_ << "\" "sv;
    if(!font_family_.empty())
        out << "font-family=\""sv << font_family_ << "\" "sv;
    if(!font_weight_.empty())
        out << "font-weight=\""sv << font_weight_ << "\" "sv;
    out << ">"sv;
    out << data_;
    out << "</text>"sv;
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for(auto& obj : objects_) {
        obj->Render({out, 2, 2});
    }
    out << "</svg>"sv << std::endl;
}

}  // namespace svg
