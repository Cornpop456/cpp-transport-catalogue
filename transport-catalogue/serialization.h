#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include <transport_catalogue.pb.h>

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace serialize {


struct Settings {
    std::filesystem::path file;
};


class Serializator final {
public:
    using ProtoTransportCatalogue = proto_catalogue::TransportCatalogue;
    using TransportCatalogue = transport::TransportCatalogue;

    Serializator(const Settings& settings) : settings_(settings) {};

    void SaveTransportCatalogue(const TransportCatalogue& catalogue);
    void SaveRenderSettings(transport::renderer::RenderSettings render_settings);
    void SaveTransportRouter(const route::TransportRouter &router);

    bool Serialize();

    bool Deserialize(TransportCatalogue& catalogue, 
        std::optional<transport::renderer::RenderSettings>& result_settings, 
        std::unique_ptr<route::TransportRouter> &router);


private:
    void SaveStops(const TransportCatalogue& catalogue);
    void LoadStops(TransportCatalogue& catalogue);

    void SaveBuses(const TransportCatalogue& catalogue);
    void LoadBuses(TransportCatalogue& catalogue);

    void SaveBusStops(const transport::Bus& bus, proto_catalogue::Bus& proto_bus, const TransportCatalogue& catalogue);
    void LoadBus(TransportCatalogue& catalogue, const proto_catalogue::Bus& proto_bus) const;

    void SaveDistances(const TransportCatalogue& catalogue);
    void LoadDistances(TransportCatalogue& catalogue) const;

    void LoadRenderSettings(std::optional<transport::renderer::RenderSettings>& result_settings) const;

    void SaveTransportRouterSettings(const route::RouteSettings& routing_settings);
    void SaveGraph(const route::TransportRouter::Graph& graph);
    void SaveRouter(const std::unique_ptr<route::TransportRouter::Router>& router);

    void LoadTransportRouter(const TransportCatalogue& catalogue,
        std::unique_ptr<route::TransportRouter>& transport_router);

    void LoadTransportRouterSettings(route::RouteSettings& routing_settings) const;
    void LoadGraph(const TransportCatalogue& catalogue, route::TransportRouter::Graph& graph);
    void LoadRouter(const TransportCatalogue& catalogue,
        std::unique_ptr<route::TransportRouter::Router>& router);

    proto_graph::RouteWeight MakeProtoWeight(const route::RouteWeight& weight) const;
    route::RouteWeight MakeWeight(const TransportCatalogue& catalogue,
        const proto_graph::RouteWeight& proto_weight) const;

    static proto_catalogue::Coordinates MakeProtoCoordinates(const geo::Coordinates& coordinates);
    static proto_svg::Point MakeProtoPoint(const svg::Point& point);
    static proto_svg::Color MakeProtoColor(const svg::Color& color);

    static geo::Coordinates MakeCoordinates(const proto_catalogue::Coordinates& proto_coordinates);
    static svg::Point MakePoint(const proto_svg::Point& proto_point);
    static svg::Color MakeColor(const proto_svg::Color& proto_color);

    Settings settings_;
    ProtoTransportCatalogue proto_catalogue_;

    std::unordered_map<int, std::string_view> bus_name_by_id_;
    std::unordered_map<std::string_view, int> bus_id_by_name_;
};

} // serialize