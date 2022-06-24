#include "map_renderer.h"

using namespace std;

namespace transport {

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

MapRenderer::MapRenderer(const SphereProjector& proj, const RenderSettings& settings) 
    :  proj_(proj), settings_(settings) {

}

void MapRenderer::AddBusToSvg(const Bus* bus) {
    if (bus->bus_stops.size() == 0) {
        return;
    }

    RouteLine line{bus, 
        proj_,
        settings_.color_palette[bus_index_ % settings_.color_palette.size()], 
        settings_.line_width};

    line.Draw(svg_doc_);

    ++bus_index_;
}

const svg::Document& MapRenderer::GetSvgDoc() {
    return svg_doc_;
}

} // transport