#include <sstream>

#include "svg.h"

namespace svg {

using namespace std::literals;

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
    
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(std::move(point));
    
    return *this;
}
    
void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    
    out << "<polyline points=\""sv;
        
    for (size_t i = 0; i < points_.size(); ++i) {
        if (i > 0) {
            out << " "sv;
        }
        
        out << points_[i].x << ","sv << points_[i].y;
    }
    
    out << "\" ";
    
    RenderAttrs(out);
        
    out << "/>";
}
    
// ---------- Text ------------------
    
std::string escape(std::string_view src) {
    std::stringstream dst;
    for (char ch : src) {
        switch (ch) {
            case '&': dst << "&amp;"; break;
            case '\'': dst << "&apos;"; break;
            case '"': dst << "&quot;"; break;
            case '<': dst << "&lt;"; break;
            case '>': dst << "&gt;"; break;
            default: dst << ch; break;
        }
    }
    return dst.str();
}
    
Text& Text::SetPosition(Point pos) {
    pos_ = std::move(pos);
    
    return *this;
}
    
Text& Text::SetOffset(Point offset) {
    offset_ = std::move(offset);
    
    return *this;
}
    
Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    
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
    data_ = std::move(escape(data));
    
    return *this;
}
    
void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
    out << "dx=\""sv << offset_.x  << "\" dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << font_size_ << "\"";
    if (font_family_) {
        out << " font-family=\""sv << *font_family_ << "\""sv;
    }
    
    if (font_weight_) {
        out << " font-weight=\""sv << *font_weight_  << "\""sv;
    }
    
    RenderAttrs(out);
    
    out << ">"sv;
    out << data_ << "</text>"sv;
}
    
// ---------- Document ------------------
    
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    svg_objects_.emplace_back(std::move(obj));
}
    
void Document::Render(std::ostream& out) const {
    RenderContext ctx(out, 2, 2);
    
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    
    for (auto& svg_obj : svg_objects_) {
        svg_obj->Render(ctx);
    }
    
    out << "</svg>"sv; 
}

void ColorPrinter::operator()(std::monostate) const {
    out << "none"sv;
}
void ColorPrinter::operator()(std::string color) const {
    out << color;
}
void ColorPrinter::operator()(Rgb color) const {
    out << "rgb("sv << int(color.red) << ","sv << int(color.green)
        << ","sv << int(color.blue) << ")"sv;
}
void ColorPrinter::operator()(Rgba color) const {
    out << "rgba("sv << int(color.red) << ","sv << int(color.green)
        << ","sv << int(color.blue) << ","sv << color.opacity << ")"sv;
}
    
std::ostream& operator<<(std::ostream &out, Color color) {
    std::visit(ColorPrinter{out}, color);
    return out;
}
    
}  // namespace svg