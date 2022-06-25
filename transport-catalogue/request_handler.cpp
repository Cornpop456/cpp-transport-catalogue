#include <sstream>

#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

using namespace std;

namespace transport {

 RequestHandler::RequestHandler(const TransportCatalogue& db, renderer::MapRenderer& renderer) 
    : db_(db), renderer_(renderer) {

}

optional<BusStat> RequestHandler::GetBusStat(const string& bus_name) const {
    return db_.GetBusStat(bus_name);
}

const set<string_view>* RequestHandler::GetBusesThroughStop(const string& stop_name) const {
    return db_.GetBusesThroughStop(stop_name);
}

const svg::Document& RequestHandler::RenderMap() const {
    renderer_.AddLinesToSvg();
    renderer_.AddBusLabelsToSvg();
    renderer_.AddStopSymToSvg();
    renderer_.AddStopLabelsToSvg();

    return renderer_.GetSvgDoc();
}

json::Document RequestHandler::GetJsonResponse(const json::Array& requests) const {
    json::Array arr;
    arr.reserve(requests.size());

    for (const auto& item : requests) {

        const auto& dict = item.AsMap();

        int id = dict.at("id"s).AsInt();
        const string& type = dict.at("type"s).AsString();

        if (type == "Bus"s) {
            const string& name = dict.at("name"s).AsString();
            const auto bus_stat_opt = GetBusStat(name);

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
            auto stop_buses = GetBusesThroughStop(name);

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

            const auto& svg_doc = RenderMap();
            svg_doc.Render(map_string);

            json_svg_map["map"s] = json::Node{map_string.str()};

            arr.push_back(move(json_svg_map));

        } else {
            throw invalid_argument("wrong query to catalogue"s);
        }

    }

    return json::Document(arr);
}

} // transport