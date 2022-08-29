#pragma once

#include <memory>

#include "json_builder.h"
#include "map_renderer.h"
#include "serialization.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace transport {

class RequestHandler {
public:
    using Route = route::TransportRouter::TransportRoute;

    RequestHandler(const TransportCatalogue& db);

    std::optional<BusStat> GetBusStat(const std::string& bus_name) const;

    const std::set<std::string_view>* GetBusesThroughStop(const std::string& stop_name) const;

    const svg::Document& RenderMap() const;
    std::optional<RequestHandler::Route> BuildRoute(const std::string &from, const std::string &to) const;

    json::Document GetJsonResponse(const json::Array& requests) const;


    bool SetRouter() const;
    bool ResetRouter() const;
    void SetRenderer(renderer::RenderSettings render_settings);

    void Serialize(serialize::Settings settings, 
        std::optional<renderer::RenderSettings> render_settings, 
        std::optional<route::RouteSettings> route_settings);

    void Deserialize(serialize::Settings settings);

private:
    const TransportCatalogue& db_;

    mutable std::unique_ptr<route::TransportRouter> router_;
    std::unique_ptr<renderer::MapRenderer> renderer_;

    std::optional<route::RouteSettings> routing_settings_;
};


} // transport