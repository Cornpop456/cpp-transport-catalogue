#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include <transport_catalogue.pb.h>

#include "transport_catalogue.h"


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

    bool Serialize();

    bool Deserialize(TransportCatalogue& catalogue);


private:
    void SaveStops(const TransportCatalogue& catalogue);
    void LoadStops(TransportCatalogue& catalogue);

    void SaveBuses(const TransportCatalogue& catalogue);
    void LoadBuses(TransportCatalogue& catalogue);

    void SaveBusStops(const transport::Bus& bus, proto_catalogue::Bus& proto_bus);
    void LoadBus(TransportCatalogue& catalogue, const proto_catalogue::Bus& proto_bus) const;

    void SaveDistances(const TransportCatalogue& catalogue);
    void LoadDistances(TransportCatalogue& catalogue) const;

    static proto_catalogue::Coordinates MakeProtoCoordinates(const geo::Coordinates& coordinates);
    static geo::Coordinates MakeCoordinates(const proto_catalogue::Coordinates& proto_coordinates);

    Settings settings_;

    ProtoTransportCatalogue proto_catalogue_;

    std::unordered_map<int, std::string_view> stop_name_by_id_;
    std::unordered_map<std::string_view, int> stop_id_by_name_;
    std::unordered_map<int, std::string_view> bus_name_by_id_;
    std::unordered_map<std::string_view, int> bus_id_by_name_;
};

} // serialize