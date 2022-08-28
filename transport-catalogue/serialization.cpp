#include <fstream>
#include <iostream>

#include "serialization.h"

namespace serialize {

void Serializator::SaveTransportCatalogue(const TransportCatalogue& catalogue) {
    SaveStops(catalogue);
    SaveBuses(catalogue);
    SaveDistances(catalogue);
}

void Serializator::SaveRenderSettings(transport::renderer::RenderSettings render_settings) {
    auto proto_settings = proto_catalogue_.mutable_render_settings();

    proto_settings->set_width(render_settings.width);
    proto_settings->set_height(render_settings.height);

    proto_settings->set_padding(render_settings.padding);

    proto_settings->set_line_width(render_settings.line_width);
    proto_settings->set_stop_radius(render_settings.stop_radius);

    proto_settings->set_bus_label_font_size(render_settings.bus_label_font_size);
    *proto_settings->mutable_bus_label_offset() = MakeProtoPoint(render_settings.bus_label_offset);

    proto_settings->set_stop_label_font_size(render_settings.stop_label_font_size);
    *proto_settings->mutable_stop_label_offset() = MakeProtoPoint(render_settings.stop_label_offset);

    *proto_settings->mutable_underlayer_color() = MakeProtoColor(render_settings.underlayer_color);
    proto_settings->set_underlayer_width(render_settings.underlayer_width);

    for (auto &color : render_settings.color_palette) {
        *proto_settings->add_color_palette() = MakeProtoColor(color);
    }
}

bool Serializator::Serialize() {
    std::ofstream out_file(settings_.file, std::ios::binary);
    
    if (!out_file.is_open ()) {
            return false;
    }

    proto_catalogue_.SerializeToOstream(&out_file);
  
    return true;
}

bool Serializator::Deserialize(TransportCatalogue& catalogue, 
    std::optional<transport::renderer::RenderSettings>& result_settings) {
    
    std::ifstream in_file(settings_.file, std::ios::binary);
    
    if (!in_file.is_open() || !proto_catalogue_.ParseFromIstream(&in_file)) {
        return false;
    }

    LoadStops(catalogue);
    LoadDistances(catalogue);
    LoadBuses(catalogue);

    LoadRenderSettings(result_settings);

    return true;
}

void Serializator::SaveStops(const TransportCatalogue& catalogue) {
    auto &stops = catalogue.GetStops();
    
    uint32_t id = 0;

    for (auto [name, stop] : stops) {
        proto_catalogue::Stop proto_stop;
        proto_stop.set_id(id);
        proto_stop.set_name(stop->name);
        *proto_stop.mutable_coordinates() = MakeProtoCoordinates(stop->coordinates);
        stop_id_by_name_.insert({name, id++});
        *proto_catalogue_.mutable_catalogue()->add_stop() = std::move(proto_stop);
    }
}

void Serializator::SaveBuses(const TransportCatalogue &catalogue) {
    auto &routes = catalogue.GetBuses();
    uint32_t id = 0;
    
    for (auto [name, bus] : routes) {
        proto_catalogue::Bus proto_bus;
        proto_bus.set_id(id);
        proto_bus.set_name(bus->name);
        proto_bus.set_circular(bus->circular);
        SaveBusStops(*bus, proto_bus);
        bus_id_by_name_.insert({name, id++});
        *proto_catalogue_.mutable_catalogue()->add_bus() = std::move(proto_bus);
    }
}

void Serializator::SaveBusStops(const transport::Bus& bus,
    proto_catalogue::Bus& proto_bus) {
    
    for (auto stop : bus.bus_stops) {
        proto_bus.add_stop_id(stop_id_by_name_.at(stop->name));
    }
}

void Serializator::SaveDistances(const TransportCatalogue& catalogue) {
    auto &distances = catalogue.GetDistances();
    
    for (auto &[from_to, length] : distances) {
        proto_catalogue::Distance proto_distance;
        proto_distance.set_stop_id_from(stop_id_by_name_.at(from_to.first->name));
        proto_distance.set_stop_id_to(stop_id_by_name_.at(from_to.second->name));
        proto_distance.set_length(length);
        *proto_catalogue_.mutable_catalogue()->add_distance() = std::move(proto_distance);
    }
}

proto_catalogue::Coordinates Serializator::MakeProtoCoordinates(const geo::Coordinates& coordinates) {
    proto_catalogue::Coordinates proto_coordinates;
    proto_coordinates.set_lat(coordinates.lat);
    proto_coordinates.set_lng(coordinates.lng);
    return proto_coordinates;
}

proto_svg::Point Serializator::MakeProtoPoint(const svg::Point& point) {
    proto_svg::Point proto_point;
    proto_point.set_x(point.x);
    proto_point.set_y(point.y);
    return proto_point;
}

proto_svg::Color Serializator::MakeProtoColor(const svg::Color& color) {
    proto_svg::Color proto_color;

    if (std::holds_alternative<std::string>(color)) {
        proto_color.set_string_color(std::get<std::string>(color));
    } else if (std::holds_alternative<svg::Rgb>(color)) {
        auto &rgb = std::get<svg::Rgb>(color);
        auto rgb_color = proto_color.mutable_rgb_color();

        rgb_color->set_r(rgb.red);
        rgb_color->set_g(rgb.green);
        rgb_color->set_b(rgb.blue);
    } else if (std::holds_alternative<svg::Rgba>(color)) {
        auto &rgba = std::get<svg::Rgba>(color);
        auto rgba_color = proto_color.mutable_rgba_color();

        rgba_color->set_r(rgba.red);
        rgba_color->set_g(rgba.green);
        rgba_color->set_b(rgba.blue);
        rgba_color->set_o(rgba.opacity);
    }

    return proto_color;
}

geo::Coordinates Serializator::MakeCoordinates(const proto_catalogue::Coordinates& proto_coordinates) {
    geo::Coordinates coordinates;
    coordinates.lat = proto_coordinates.lat();
    coordinates.lng = proto_coordinates.lng();
    return coordinates;
}

svg::Point Serializator::MakePoint(const proto_svg::Point& proto_point) {
    svg::Point point;
    point.x = proto_point.x();
    point.y = proto_point.y();
    return point;
}


svg::Color Serializator::MakeColor(const proto_svg::Color& proto_color) {
    svg::Color color;

    switch (proto_color.color_case()) {
        case proto_svg::Color::kStringColor:
            color = proto_color.string_color();
        break;
        case proto_svg::Color::kRgbColor:
        {
            auto& proto_rgb = proto_color.rgb_color();
            color = svg::Rgb(proto_rgb.r(), proto_rgb.g(), proto_rgb.b());
        }
        break;
        case proto_svg::Color::kRgbaColor:
        {
            auto& proto_rgba = proto_color.rgba_color();
            color = svg::Rgba(proto_rgba.r(), proto_rgba.g(), proto_rgba.b(), proto_rgba.o());
        }
        break;
        default:
            color = svg::NoneColor;
        break;
    }

    return color;
}

void Serializator::LoadStops(TransportCatalogue& catalogue) {
    auto stops_count = proto_catalogue_.catalogue().stop_size();
    
    for (int i = 0; i < stops_count; ++i) {
        auto &proto_stop = proto_catalogue_.catalogue().stop(i);

        auto coords = MakeCoordinates(proto_stop.coordinates());

        catalogue.AddStop({proto_stop.name(), coords.lat, coords.lng});
        stop_name_by_id_.insert({proto_stop.id(), proto_stop.name()});
    }
}

void Serializator::LoadBuses(TransportCatalogue &catalogue) {
    auto buses_count = proto_catalogue_.catalogue().bus_size();
    
    for (int i = 0; i < buses_count; ++i) {
        auto& proto_bus = proto_catalogue_.catalogue().bus(i);
        LoadBus(catalogue, proto_bus);
        bus_name_by_id_.insert({proto_bus.id(), proto_bus.name()});
    }
}

void Serializator::LoadBus(TransportCatalogue& catalogue,
                             const proto_catalogue::Bus& proto_bus) const {
    auto stops_count = proto_bus.stop_id_size();
    
    std::vector<std::string> stops;
    
    stops.reserve(stops_count);
    
    for(int i = 0; i < stops_count; ++i) {
        auto stop_name = stop_name_by_id_.at(proto_bus.stop_id(i));
        stops.push_back(std::string(stop_name));
    }

    catalogue.AddBus({proto_bus.name(), move(stops), proto_bus.circular()});
}

void Serializator::LoadDistances(TransportCatalogue& catalogue) const {
    std::unordered_map<std::string, std::unordered_map<std::string, int>> d_map;

    auto distances_count = proto_catalogue_.catalogue().distance_size();
    
    for (int i = 0; i < distances_count; ++i) {
        auto &proto_distance = proto_catalogue_.catalogue().distance(i);
        
        auto stop_from = std::string(stop_name_by_id_.at(proto_distance.stop_id_from()));
        auto stop_to = std::string(stop_name_by_id_.at(proto_distance.stop_id_to()));

        d_map[stop_from][stop_to] = proto_distance.length();
    }

    for (auto& [from, mp_to] : d_map) {
        catalogue.AddDistances({move(from), move(mp_to)});
    }
}

void Serializator::LoadRenderSettings(std::optional<transport::renderer::RenderSettings>& result_settings) const {
    if (!proto_catalogue_.has_render_settings()) {
        return;
    }

    auto& proto_settings = proto_catalogue_.render_settings();

    transport::renderer::RenderSettings settings;

    settings.width = proto_settings.width();

    settings.height = proto_settings.height();

    settings.padding = proto_settings.padding();

    settings.line_width = proto_settings.line_width();
    settings.stop_radius = proto_settings.stop_radius();

    settings.bus_label_font_size = proto_settings.bus_label_font_size();
    settings.bus_label_offset = MakePoint(proto_settings.bus_label_offset());

    settings.stop_label_font_size = proto_settings.stop_label_font_size();
    settings.stop_label_offset = MakePoint(proto_settings.stop_label_offset());

    settings.underlayer_color = MakeColor(proto_settings.underlayer_color());
    settings.underlayer_width = proto_settings.underlayer_width();

    auto color_pallete_count = proto_settings.color_palette_size();
    settings.color_palette.clear();
    settings.color_palette.reserve(color_pallete_count);

    for (int i = 0; i < color_pallete_count; ++i) {
        settings.color_palette.push_back(MakeColor(proto_settings.color_palette(i)));
    }

    result_settings = settings;
}



} // serialize