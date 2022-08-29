#pragma once

#include <iostream>

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "json.h"
#include "request_handler.h"
#include "serialization.h"
#include "transport_router.h"

namespace transport {

class JsonReader {
private:
    json::Document json_doc_{json::Node{nullptr}};

    const json::Array& GetBaseRequests() const;

    const json::Dict& GetRenderSettingsJson() const;

    const json::Array& GetStatRequests() const; 

    svg::Color GetColorFromNode(const json::Node& n) const;

    renderer::RenderSettings DictToRenderSettings(const json::Dict& settings_dict) const;

    parsed::Bus DictToBus(const json::Dict& bus_dict) const;

    std::pair<parsed::Stop, parsed::Distances> DictToStopDists(const json::Dict& stop_dict) const;

public:
    JsonReader(std::istream& input);

    route::RouteSettings GetRouteSettings() const;

    serialize::Settings GetSerializeSettings() const;

    std::optional<renderer::RenderSettings> GetRenderSettings() const;

    std::optional<route::RouteSettings> GetRouteSettingsOpt() const;

    bool HasRenderSettings() const; 

    void FillCatalogue(TransportCatalogue& catalogue) const;

    void PrintJsonResponse(const RequestHandler& handler, std::ostream& out) const;
};

} // transport