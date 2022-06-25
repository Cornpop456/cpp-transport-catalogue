#include "map_renderer.h"

using namespace std;

namespace transport {

namespace renderer {

namespace map_objects {

RouteLine::RouteLine(const Bus* bus, 
    const SphereProjector& proj, 
    svg::Color color, 
    double stroke_width) 
    
    : bus_(bus), proj_(proj), color_(color), stroke_width_(stroke_width) {

}

void RouteLine::Draw(svg::ObjectContainer& container) const {
    svg::Polyline pol;

    for (const auto stop : bus_->bus_stops) {
         pol.AddPoint(proj_(stop->coordinates));
    }

    if (!bus_->circular && bus_->bus_stops.size() > 1) {
        for (int i = bus_->bus_stops.size() - 2; i >= 0; --i) {
            pol.AddPoint(proj_(bus_->bus_stops[i]->coordinates));
        }
    } 
    
    pol.SetFillColor(svg::NoneColor)
       .SetStrokeColor(color_)
       .SetStrokeWidth(stroke_width_)
       .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
       .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    container.Add(pol);
}

BusLabel::BusLabel(const Bus* bus,
    const SphereProjector& proj,  
    const RenderSettings& settings,
    int color_idx) 
    
    : bus_(bus), proj_(proj), settings_(settings), color_idx_(color_idx) {

}

void BusLabel::Draw(svg::ObjectContainer& container) const {
    svg::Text base;
    
    base.SetPosition(proj_(bus_->bus_stops.front()->coordinates))
        .SetOffset(settings_.bus_label_offset)
        .SetFontSize(settings_.bus_label_font_size)
        .SetFontFamily("Verdana"s)
        .SetFontWeight("bold"s)
        .SetData(bus_->name);
    
    svg::Text under = base;
    svg::Text actual = move(base);
       
    under.SetFillColor(settings_.underlayer_color)
            .SetStrokeColor(settings_.underlayer_color)
            .SetStrokeWidth(settings_.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    actual.SetFillColor(settings_.color_palette[color_idx_ % settings_.color_palette.size()]);

    container.Add(under);
    container.Add(actual);

    if (!bus_->circular && bus_->bus_stops.front() != bus_->bus_stops.back()) {
        under.SetPosition(proj_(bus_->bus_stops.back()->coordinates));
        actual.SetPosition(proj_(bus_->bus_stops.back()->coordinates));

        container.Add(under);
        container.Add(actual);
    }
}

StopSymbols::StopSymbols(const std::vector<const Stop*>& stops,
        const SphereProjector& proj,  
        
        double stop_radius) : stops_(stops), proj_(proj), stop_radius_(stop_radius) {
}

void StopSymbols::Draw(svg::ObjectContainer& container) const {
    for (const auto stop : stops_) {
        svg::Circle sym;

        sym.SetCenter(proj_(stop->coordinates))
           .SetRadius(stop_radius_)
           .SetFillColor("white"s);

        container.Add(sym);
    }
}

StopLabels::StopLabels(const std::vector<const Stop*>& stops,
        const SphereProjector& proj,  
        const RenderSettings& settings)

        : stops_(stops), proj_(proj), settings_(settings) {

}

void StopLabels::Draw(svg::ObjectContainer& container) const {
    svg::Text base;

    base.SetOffset(settings_.stop_label_offset)
        .SetFontSize(settings_.stop_label_font_size)
        .SetFontFamily("Verdana"s);
    
    svg::Text under = base;
    svg::Text actual = move(base);
    
    under.SetFillColor(settings_.underlayer_color)
         .SetStrokeColor(settings_.underlayer_color)
         .SetStrokeWidth(settings_.underlayer_width)
         .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
         .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    actual.SetFillColor("black"s);
         
    for (const auto stop : stops_) {
        under.SetPosition(proj_(stop->coordinates))
             .SetData(stop->name);

        actual.SetPosition(proj_(stop->coordinates))
              .SetData(stop->name);

        container.Add(under);
        container.Add(actual);
    }
}

} // map_objects

MapRenderer::MapRenderer(SphereProjector proj, 
        RenderSettings settings, 
        vector<const Bus*> buses, 
        vector<const Stop*> stops)

    :  proj_(proj), settings_(settings), buses_(buses), stops_(stops) {
}

void MapRenderer::AddLinesToSvg() {
    color_index_ = 0;

    for (const auto bus : buses_) {
        
        if (bus->bus_stops.size() == 0) {
            continue;
        }

        map_objects::RouteLine line{
            bus, 
            proj_,
            settings_.color_palette[color_index_ % settings_.color_palette.size()], 
            settings_.line_width};

        line.Draw(svg_doc_);

        ++color_index_;
    }
}

void MapRenderer::AddBusLabelsToSvg() {
    color_index_ = 0;

    for (const auto bus : buses_) {
        if (bus->bus_stops.size() == 0) {
            continue;
        }

        map_objects::BusLabel label{
            bus, 
            proj_,
            settings_,
            color_index_};

        label.Draw(svg_doc_);

        ++color_index_;
    }
}

void MapRenderer::AddStopSymToSvg() {
        if (stops_.size() == 0) {
            return;
        }

        map_objects::StopSymbols syms{
            stops_, 
            proj_,
            settings_.stop_radius};

        syms.Draw(svg_doc_);
}

void MapRenderer::AddStopLabelsToSvg() {
        if (stops_.size() == 0) {
            return;
        }

        map_objects::StopLabels labels{
            stops_,
            proj_,
            settings_
        };

        labels.Draw(svg_doc_);
}

const svg::Document& MapRenderer::GetSvgDoc() const {
    return svg_doc_;
}

} // renderer

} // transport