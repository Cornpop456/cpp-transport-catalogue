#include <sstream>

#include "json.h"
#include "json_reader.h"
#include "request_handler.h"

using namespace std;

namespace transport {

namespace {

json::Document ReadJSON(std::istream& input) {
    return json::Load(input);
}

const json::Array& GetBaseRequests(const json::Document& doc) {
    return doc.GetRoot().AsMap().at("base_requests"s).AsArray();
}

const json::Dict& GetRenderSettings(const json::Document& doc) {
    return doc.GetRoot().AsMap().at("render_settings"s).AsMap();
}

const json::Array& GetStatRequests(const json::Document& doc) {
    return doc.GetRoot().AsMap().at("stat_requests"s).AsArray();
}

 svg::Color GetColorFromNode(const json::Node& n) {
    if (n.IsString()) {
        return n.AsString();
    } 
    
    const json::Array& arr = n.AsArray();

    if (arr.size() == 3) {
        return svg::Rgb{
            static_cast<uint8_t>(arr[0].AsInt()), 
            static_cast<uint8_t>(arr[1].AsInt()), 
            static_cast<uint8_t>(arr[2].AsInt())};
    } 
    
    return svg::Rgba{
        static_cast<uint8_t>(arr[0].AsInt()), 
        static_cast<uint8_t>(arr[1].AsInt()), 
        static_cast<uint8_t>(arr[2].AsInt()), 
        arr[3].AsDouble()};
}

renderer::RenderSettings DictToRenderSettings(const json::Dict& settings_dict) {
    renderer::RenderSettings settings;
    
    settings.width = settings_dict.at("width"s).AsDouble();
    settings.height = settings_dict.at("height"s).AsDouble();
    settings.padding = settings_dict.at("padding"s).AsDouble();
    settings.line_width = settings_dict.at("line_width"s).AsDouble();
    settings.stop_radius = settings_dict.at("stop_radius"s).AsDouble();
    settings.bus_label_font_size = settings_dict.at("bus_label_font_size"s).AsInt();

    const auto& blo = settings_dict.at("bus_label_offset"s).AsArray();

    settings.bus_label_offset = svg::Point{blo[0].AsDouble(), blo[1].AsDouble()};
    settings.stop_label_font_size = settings_dict.at("stop_label_font_size"s).AsInt();

    const auto& slo = settings_dict.at("stop_label_offset"s).AsArray();

    settings.stop_label_offset = svg::Point{slo[0].AsDouble(), slo[1].AsDouble()};

    settings.underlayer_color = GetColorFromNode(settings_dict.at("underlayer_color"s));

    settings.underlayer_width  = settings_dict.at("underlayer_width"s).AsDouble();

    for (const json::Node& n: settings_dict.at("color_palette"s).AsArray()) {
        settings.color_palette.push_back(GetColorFromNode(n));
    }

    return settings;
}


    
parsed::Bus DictToBus(const json::Dict& bus_dict) {
    parsed::Bus bus;

    bus.name = move(const_cast<string&>(bus_dict.at("name"s).AsString()));
    bus.circular = bus_dict.at("is_roundtrip"s).AsBool();

    for (const auto& str_node: bus_dict.at("stops"s).AsArray()) {
        bus.stops.push_back(move(const_cast<string&>(str_node.AsString())));
    }

    return bus;
}

std::pair<parsed::Stop, parsed::Distances> DictToStopDists(const json::Dict& stop_dict) {
    parsed::Stop p_s;
    parsed::Distances p_d;

    p_s.name = stop_dict.at("name"s).AsString();
    p_d.from = move(const_cast<string&>(stop_dict.at("name"s).AsString()));

    p_s.lat = stop_dict.at("latitude"s).AsDouble();
    p_s.lng = stop_dict.at("longitude"s).AsDouble();

    for (const auto& [to, dist]: stop_dict.at("road_distances"s).AsMap()) {
        p_d.d_map.emplace(move(const_cast<string&>(to)), dist.AsInt());
    }

    return {p_s, p_d};
}

renderer::MapRenderer GetRenderer(const TransportCatalogue& catalogue, renderer::RenderSettings settings) {
    vector<geo::Coordinates> all_coords;

    vector<const Bus*> buses;

    auto comp = [] (const Stop* a, const Stop* b) {
        return a->name < b->name;
    };

    set<const Stop*, decltype(comp)> stops(comp);

    for (auto el : *catalogue.GetBusNames()) {
                
        buses.push_back(catalogue.GetBus(el));
        
        const auto& bus_stops = catalogue.GetBus(el)->bus_stops;

        for (const auto stop : bus_stops) {
            all_coords.push_back(stop->coordinates);
            stops.insert(stop);
        }
    }

    SphereProjector proj(all_coords.begin(), all_coords.end(), 
        settings.width, 
        settings.height, 
        settings.padding);

    all_coords.clear();

    vector<const Stop*> stops_vec(make_move_iterator(stops.begin()), 
        make_move_iterator(stops.end()));

    renderer::MapRenderer map_renderer(move(proj), move(settings), move(buses), move(stops_vec));

    return map_renderer;
}

void FillCatalogue(TransportCatalogue& catalogue, const json::Array& requests) {
    vector<parsed::Bus> add_bus_deferred;
    vector<parsed::Distances> add_dists_deferred;

    for (const auto& item : requests) {

        const auto& dict = item.AsMap();
        
        const string& type = dict.at("type"s).AsString();

        if (type == "Stop"s) {
            auto [parsed_stop, parsed_distances] = DictToStopDists(dict);

            if (parsed_distances.d_map.size() > 0) {
                add_dists_deferred.push_back(move(parsed_distances));
            }

            catalogue.AddStop(parsed_stop);
        } else if (type == "Bus"s) {
            add_bus_deferred.push_back(DictToBus(dict));
        } else {
            throw invalid_argument("wrong query to catalogue"s);
        }
    }

    for (const auto& d : add_dists_deferred) {
        catalogue.AddDistances(d);
    }

    for (const auto& bus : add_bus_deferred) {
        catalogue.AddBus(bus);
    }
}

void BuildSvgMap(const RequestHandler& handler, std::ostream& out) {
    const svg::Document& svg_doc = handler.RenderMap();

    svg_doc.Render(out);
}

void PrintOutput(const RequestHandler& handler, const json::Array& requests, std::ostream& out) {
    json::Array arr;
    arr.reserve(requests.size());
    
    for (const auto& item : requests) {

        const auto& dict = item.AsMap();

        int id = dict.at("id"s).AsInt();
        const string& type = dict.at("type"s).AsString();

        if (type == "Bus"s) {
            const string& name = dict.at("name"s).AsString();
            const auto bus_stat_opt = handler.GetBusStat(name);

            if (!bus_stat_opt) {
                arr.push_back(json::Dict{{"request_id"s, id}, {"error_message"s, "not found"s}});

                continue;
            }

            const auto& bus_stat = bus_stat_opt.value();
            json::Dict json_stat;
            json_stat["request_id"s] = json::Node{id};
            json_stat["route_length"s] = json::Node{static_cast<int>(bus_stat.length)};
            json_stat["curvature"s] = json::Node{bus_stat.curvature};
            json_stat["stop_count"s] = json::Node{bus_stat.all_stops};
            json_stat["unique_stop_count"s] = json::Node{bus_stat.unique_stops};

            arr.push_back(move(json_stat));
        } else if (type == "Stop"s) {
            const string& name = dict.at("name"s).AsString();
            auto stop_buses = handler.GetBusesThroughStop(name);

            if (!stop_buses) {
                arr.push_back(json::Dict{{"request_id"s, id}, {"error_message"s, "not found"s}});

                continue;
            }

            json::Dict json_stop_buses;
            json_stop_buses["request_id"s] = json::Node{id};

            json::Array buses;
            buses.reserve(stop_buses->size());

            for (string_view bus: *stop_buses) {
                buses.push_back(json::Node{string(bus)});
            }

            json_stop_buses["buses"s] = move(buses);

            arr.push_back(move(json_stop_buses));
        } else if (type == "Map"s) {
            json::Dict json_svg_map;
            json_svg_map["request_id"s] = json::Node{id};

            stringstream map_string;

            BuildSvgMap(handler, map_string);

            json_svg_map["map"s] = json::Node{map_string.str()};

            arr.push_back(move(json_svg_map));

        } else {
            throw invalid_argument("wrong query to catalogue"s);
        }

    }

    json::Print(json::Document(arr), out);
}

} // namespace


namespace json_reader {

void ProcessJSON(TransportCatalogue& catalogue, std::istream& in, std::ostream& out,
    Format out_format) {

    auto json = ReadJSON(in);
    
    FillCatalogue(catalogue, GetBaseRequests(json));

    auto settings = DictToRenderSettings(GetRenderSettings(json));

    renderer::MapRenderer map_renderer = GetRenderer(catalogue, move(settings));

    RequestHandler handler(catalogue, map_renderer);

    switch (out_format) {
        case Format::JSON:
            PrintOutput(handler, GetStatRequests(json), out);
            break;
        case Format::SVG:
            BuildSvgMap(handler, out);
    }
}

} // json_reader

} // transport