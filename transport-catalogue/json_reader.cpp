#include "json_reader.h"

using namespace std;

namespace transport {

JsonReader::JsonReader(std::istream& input) : json_doc_(json::Load(input)) {

}

const json::Array& JsonReader::GetBaseRequests() const {
    return json_doc_.GetRoot().AsDict().at("base_requests"s).AsArray();
}

const json::Dict& JsonReader::GetRenderSettingsJson() const {
    return json_doc_.GetRoot().AsDict().at("render_settings"s).AsDict();
}

bool JsonReader::HasRenderSettings() const {
    return json_doc_.GetRoot().AsDict().count("render_settings"s) > 0;
}

const json::Array& JsonReader::GetStatRequests() const {
    return json_doc_.GetRoot().AsDict().at("stat_requests"s).AsArray();
}

route::RouteSettings JsonReader::GetRouteSettings() const {
    const json::Dict& settings =  json_doc_.GetRoot().AsDict().at("routing_settings"s).AsDict();

    return {settings.at("bus_wait_time").AsInt(), settings.at("bus_velocity").AsInt()};
}

optional<route::RouteSettings> JsonReader::GetRouteSettingsOpt() const {
    if (json_doc_.GetRoot().AsDict().count("routing_settings"s) > 0) {
        const json::Dict& settings =  json_doc_.GetRoot().AsDict().at("routing_settings"s).AsDict();

        return route::RouteSettings{settings.at("bus_wait_time").AsInt(), settings.at("bus_velocity").AsInt()};
    }

    return {};
}

serialize::Settings JsonReader::GetSerializeSettings() const {
    return {json_doc_.GetRoot().AsDict().at("serialization_settings"s).AsDict().at("file").AsString()};
}

svg::Color JsonReader::GetColorFromNode(const json::Node& n) const {
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

renderer::RenderSettings JsonReader::DictToRenderSettings(const json::Dict& settings_dict) const {
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

parsed::Bus JsonReader::DictToBus(const json::Dict& bus_dict) const {
    parsed::Bus bus;

    bus.name = move(const_cast<string&>(bus_dict.at("name"s).AsString()));
    bus.circular = bus_dict.at("is_roundtrip"s).AsBool();

    for (const auto& str_node: bus_dict.at("stops"s).AsArray()) {
        bus.stops.push_back(move(const_cast<string&>(str_node.AsString())));
    }

    return bus;
}

std::pair<parsed::Stop, parsed::Distances> JsonReader::DictToStopDists(const json::Dict& stop_dict) const {
    parsed::Stop p_s;
    parsed::Distances p_d;

    p_s.name = stop_dict.at("name"s).AsString();
    p_d.from = move(const_cast<string&>(stop_dict.at("name"s).AsString()));

    p_s.lat = stop_dict.at("latitude"s).AsDouble();
    p_s.lng = stop_dict.at("longitude"s).AsDouble();

    for (const auto& [to, dist]: stop_dict.at("road_distances"s).AsDict()) {
        p_d.d_map.emplace(move(const_cast<string&>(to)), dist.AsInt());
    }

    return {p_s, p_d};
}

optional<renderer::RenderSettings> JsonReader::GetRenderSettings() const {
    if (HasRenderSettings()) {
        return DictToRenderSettings(GetRenderSettingsJson());
    }

    return {};
 }


// Оставил наполенние каталога в JsonReader потому что иначе пришлось бы переносить всю логику разбора json запросов
// в RequestHandler, а он этим по идее не должен заниматься
void JsonReader::FillCatalogue(TransportCatalogue& catalogue) const {
    vector<parsed::Bus> add_bus_deferred;
    vector<parsed::Distances> add_dists_deferred;

    const json::Array& requests = GetBaseRequests();

    for (const auto& item : requests) {

        const auto& dict = item.AsDict();
        
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


// перенёс логику формирования json массива в RequestHandler, так как если бы он  выдавал не json, а свои структуры,
// то пришлось бы ещё раз проверять тип возвращённого значения, для формирования нужного элемента json массива
// в этой функции, а это вызвыло бы дублирование кода
void JsonReader::PrintJsonResponse(const RequestHandler& handler, std::ostream& out) const {
    const json::Array& requests = GetStatRequests();

    json::Print(handler.GetJsonResponse(requests), out);
}

} // transport