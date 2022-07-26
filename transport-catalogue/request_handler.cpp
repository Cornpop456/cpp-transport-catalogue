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

namespace {

json::Node GetNodeStop(const route::RouteData& data) {
    json::Node node_stop{ json::Builder{}.StartDict()
        .Key("stop_name"s).Value(std::string(data.stop_name))
        .Key("time"s).Value(data.bus_wait_time)
        .Key("type"s).Value("Wait"s)
        .EndDict().Build() };
        
    return node_stop;
}

json::Node GetNodeBus(const route::RouteData& data) {
    json::Node node_route{ json::Builder{}.StartDict()
        .Key("bus"s).Value(std::string(data.bus_name))
        .Key("span_count"s).Value(data.span_count)
        .Key("time"s).Value(data.motion_time)
        .Key("type"s).Value("Bus"s)
        .EndDict().Build() };
    return node_route;
}

json::Node GetNodeRoute(const std::vector<route::RouteData>& route_data) {
    json::Node data_route{ json::Builder{}.StartArray().EndArray().Build() };
    for (const route::RouteData& data : route_data) {
        if (data.type == "bus"sv) {
            const_cast<json::Array&>(data_route.AsArray()).push_back(std::move(GetNodeBus(data)));
        }
        else if (data.type == "stop"sv) {
            const_cast<json::Array&>(data_route.AsArray()).push_back(std::move(GetNodeStop(data)));
        }
        else if (data.type == "stay_here"sv) {
            return data_route;
        }
    }
    return data_route;
}

double GetTotalTime(const std::vector<route::RouteData>& route_data) {
    double total_time = 0.0;
    for (const route::RouteData& data : route_data) {
        if (data.type == "bus"sv) {
            total_time += data.motion_time;
        }
        else if (data.type == "stop"sv) {
            total_time += data.bus_wait_time;
        }
    }
    return total_time;
}

}

namespace transport {

 RequestHandler::RequestHandler(const TransportCatalogue& db, 
    const route::TransportRouter& router, 
    renderer::MapRenderer& renderer) 
    : db_(db), router_(router), renderer_(renderer) {

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
        } else if (type == "Route"s) {
            auto route_data = router_.GetRoute(dict.at("from"s).AsString(), dict.at("to"s).AsString());

            if (!route_data) {
                arr_ctx.StartDict()
                    .Key("request_id"s)
                    .Value(id)
                    .Key("error_message"s)
                    .Value("not found"s)
                    .EndDict();
                continue;
            }

            arr_ctx.StartDict()
                .Key("items"s)
                .Value(GetNodeRoute(route_data.value()).AsArray())
                .Key("request_id"s)
                .Value(id)
                .Key("total_time")
                .Value(GetTotalTime(route_data.value()))
                .EndDict();
            
        } else {
            throw invalid_argument("wrong query to catalogue"s);
        }
    }

    return json::Document(arr_ctx.EndArray().Build());
}

} // transport