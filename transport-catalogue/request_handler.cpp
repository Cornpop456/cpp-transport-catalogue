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

 RequestHandler::RequestHandler(const TransportCatalogue& db) : db_(db){
}

void RequestHandler::SetRenderer(renderer::RenderSettings settings) {
    vector<geo::Coordinates> all_coords;

    vector<const Bus*> buses;

    auto comp = [] (const Stop* a, const Stop* b) {
        return a->name < b->name;
    };

    set<const Stop*, decltype(comp)> stops(comp);

    for (auto el : *db_.GetBusNames()) {
                
        buses.push_back(db_.GetBus(el));
        
        const auto& bus_stops = db_.GetBus(el)->bus_stops;

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

    renderer_ = make_unique<renderer::MapRenderer>(move(proj), move(settings), move(buses), move(stops_vec));
}

optional<BusStat> RequestHandler::GetBusStat(const string& bus_name) const {
    return db_.GetBusStat(bus_name);
}

const set<string_view>* RequestHandler::GetBusesThroughStop(const string& stop_name) const {
    return db_.GetBusesThroughStop(stop_name);
}

const svg::Document& RequestHandler::RenderMap() const {
    renderer_->AddLinesToSvg();
    renderer_->AddBusLabelsToSvg();
    renderer_->AddStopSymToSvg();
    renderer_->AddStopLabelsToSvg();

    return renderer_->GetSvgDoc();
}

bool RequestHandler::ResetRouter() const {
    if (routing_settings_) {
        router_ = std::make_unique<route::TransportRouter>(db_, routing_settings_.value());
        return true;
    } else {
        std::cerr << "Can't find routing settings"s << std::endl;
        return false;
    }
}

bool RequestHandler::SetRouter() const {
    if (!router_) {
        return ResetRouter();
    }
    return true;
}

std::optional<RequestHandler::Route>
RequestHandler::BuildRoute(const std::string &from, const std::string &to) const {
    if (!SetRouter()) {
        return std::nullopt;
    } else {
        return router_->BuildRoute(from, to);
    }
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
            auto route_data = BuildRoute(dict.at("from"s).AsString(), dict.at("to"s).AsString());

            if (!route_data) {
                arr_ctx.StartDict()
                    .Key("request_id"s)
                    .Value(id)
                    .Key("error_message"s)
                    .Value("not found"s)
                    .EndDict();
                continue;
            }

            double total_time = 0;
            int wait_time = router_->GetSettings().bus_wait_time;
            json::Array items;

            for (const auto &edge : route_data.value()) {
                total_time += edge.total_time;

                json::Node wait_elem = json::Builder{}.StartDict().
                    Key("type"s).Value("Wait"s).
                    Key("stop_name"s).Value(std::string(edge.stop_from)).
                    Key("time"s).Value(wait_time).
                    EndDict().Build();
                
                json::Node ride_elem = json::Builder{}.StartDict().
                    Key("type"s).Value("Bus"s).
                    Key("bus"s).Value(std::string(edge.bus_name)).
                    Key("span_count"s).Value(edge.span_count).
                    Key("time"s).Value(edge.total_time - wait_time).
                    EndDict().Build();
                
                items.push_back(wait_elem);
                items.push_back(ride_elem);
            }
            
            arr_ctx.StartDict()
                .Key("request_id"s).Value(id)
                .Key("total_time"s).Value(total_time)
                .Key("items"s).Value(items)
                .EndDict();
        } else {
            throw invalid_argument("wrong query to catalogue"s);
        }
    }

    return json::Document(arr_ctx.EndArray().Build());
}

void RequestHandler::Serialize(serialize::Settings settings, 
    optional<renderer::RenderSettings> render_settings,
    optional<route::RouteSettings> route_settings) {
    
    serialize::Serializator serializator(settings);

    serializator.SaveTransportCatalogue(db_);

    if (render_settings) {
       serializator.SaveRenderSettings(move(render_settings.value())); 
    }

    if (route_settings) {
        router_ = std::make_unique<route::TransportRouter>(db_, route_settings.value());
        router_->InitRouter();
        serializator.SaveTransportRouter(*router_.get());
    }

    serializator.Serialize();
}

void RequestHandler::Deserialize(serialize::Settings settings) {
    serialize::Serializator serializator(settings);
    
    optional<renderer::RenderSettings> render_settings;

    serializator.Deserialize(const_cast<TransportCatalogue&>(db_), render_settings, router_);

    if (router_) {
        routing_settings_ = router_->GetSettings();
    }

    if (render_settings) {
        SetRenderer(render_settings.value());
    }
}

} // transport