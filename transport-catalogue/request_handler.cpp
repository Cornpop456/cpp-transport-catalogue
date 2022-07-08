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
    auto response_builder = json::Builder{};
    
    auto arr_ctx = response_builder.StartArray();
    
    for (const auto& item : requests) {

        const auto& dict = item.AsDict();

        int id = dict.at("id"s).AsInt();
        const string& type = dict.at("type"s).AsString();

        if (type == "Bus"s) {
            const string& name = dict.at("name"s).AsString();
            const auto bus_stat_opt = GetBusStat(name);

            if (!bus_stat_opt) {     
                arr_ctx.StartDict()
                    .Key("request_id"s)
                    .Value(id)
                    .Key("error_message"s)
                    .Value("not found"s)
                    .EndDict();
                continue;
            }

            const auto& bus_stat = bus_stat_opt.value();

            arr_ctx.StartDict()
                .Key("request_id"s)
                .Value(id)
                .Key("curvature"s)
                .Value(bus_stat.curvature)
                .Key("stop_count"s)
                .Value(bus_stat.all_stops)
                .Key("unique_stop_count"s)
                .Value(bus_stat.unique_stops)
                .Key("route_length"s)
                .Value(static_cast<int>(bus_stat.length))
                .EndDict();
        } else if (type == "Stop"s) {
            const string& name = dict.at("name"s).AsString();
            auto stop_buses = GetBusesThroughStop(name);

            if (!stop_buses) {
                arr_ctx.StartDict()
                    .Key("request_id"s)
                    .Value(id)
                    .Key("error_message"s)
                    .Value("not found"s)
                    .EndDict();
                continue;
            }
            
            arr_ctx.StartDict()
                .Key("request_id"s)
                .Value(id)
                .Key("buses"s)
                .StartArray();

            for (string_view bus: *stop_buses) {
                arr_ctx.Value(string(bus));
            }
            
            arr_ctx.EndArray().EndDict();        
        } else if (type == "Map"s) {
            stringstream map_string;

            const auto& svg_doc = RenderMap();
            svg_doc.Render(map_string);
                 
            arr_ctx.StartDict()
                .Key("request_id"s)
                .Value(id)
                .Key("map"s)
                .Value(map_string.str())
                .EndDict();
        } else {
            throw invalid_argument("wrong query to catalogue"s);
        }
    }

    return json::Document(arr_ctx.EndArray().Build());
}

} // transport