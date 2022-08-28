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
    RequestHandler(const TransportCatalogue& db);

    std::optional<BusStat> GetBusStat(const std::string& bus_name) const;

    const std::set<std::string_view>* GetBusesThroughStop(const std::string& stop_name) const;

    const svg::Document& RenderMap() const;

    json::Document GetJsonResponse(const json::Array& requests) const;

    void SetTransportRouter(std::unique_ptr<route::TransportRouter>&& router);
    void SetRenderer(renderer::RenderSettings render_settings);

    void Serialize(serialize::Settings settings, std::optional<renderer::RenderSettings> render_settings);
    void Deserialize(serialize::Settings settings);

private:
    const TransportCatalogue& db_;

    std::unique_ptr<route::TransportRouter> router_;
    std::unique_ptr<renderer::MapRenderer> renderer_;
};


} // transport