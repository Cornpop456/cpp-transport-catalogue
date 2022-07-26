#pragma once

#include "json_builder.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace transport {

class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& db, const route::TransportRouter& router, renderer::MapRenderer& renderer);

    std::optional<BusStat> GetBusStat(const std::string& bus_name) const;

    const std::set<std::string_view>* GetBusesThroughStop(const std::string& stop_name) const;

    const svg::Document& RenderMap() const;

    json::Document GetJsonResponse(const json::Array& requests) const;

private:
    const TransportCatalogue& db_;
    const route::TransportRouter& router_;
    renderer::MapRenderer& renderer_;

};


} // transport