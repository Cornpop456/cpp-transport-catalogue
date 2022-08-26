#include <fstream>
#include <iostream>

#include "serialization.h"

namespace serialize {

void Serializator::SaveTransportCatalogue(const TransportCatalogue& catalogue) {
    SaveStops(catalogue);
    SaveBuses(catalogue);
    SaveDistances(catalogue);
}

bool Serializator::Serialize() {
    std::ofstream out_file(settings_.file, std::ios::binary);
    
    if (!out_file.is_open ()) {
            return false;
    }

    proto_catalogue_.SerializeToOstream(&out_file);
  
    return true;
}

bool Serializator::Deserialize(TransportCatalogue& catalogue) {
    std::ifstream in_file(settings_.file, std::ios::binary);
    
    if (!in_file.is_open() || !proto_catalogue_.ParseFromIstream(&in_file)) {
        return false;
    }

    LoadStops(catalogue);
    LoadDistances(catalogue);
    LoadBuses(catalogue);

    return true;
}

void Serializator::SaveStops(const TransportCatalogue& catalogue) {
    auto &stops = catalogue.GetStops();
    
    uint32_t id = 0;

    for (auto [name, stop] : stops) {
        proto_catalogue::Stop proto_stop;
        proto_stop.set_id(id);
        proto_stop.set_name(stop->name);
        *proto_stop.mutable_coordinates() = MakeProtoCoordinates(stop->coordinates);
        stop_id_by_name_.insert({name, id++});
        *proto_catalogue_.mutable_catalogue()->add_stop() = std::move(proto_stop);
    }
}

void Serializator::SaveBuses(const TransportCatalogue &catalogue) {
    auto &routes = catalogue.GetBuses();
    uint32_t id = 0;
    
    for (auto [name, bus] : routes) {
        proto_catalogue::Bus proto_bus;
        proto_bus.set_id(id);
        proto_bus.set_name(bus->name);
        proto_bus.set_circular(bus->circular);
        SaveBusStops(*bus, proto_bus);
        bus_id_by_name_.insert({name, id++});
        *proto_catalogue_.mutable_catalogue()->add_bus() = std::move(proto_bus);
    }
}

void Serializator::SaveBusStops(const transport::Bus& bus,
    proto_catalogue::Bus& proto_bus) {
    
    for (auto stop : bus.bus_stops) {
        proto_bus.add_stop_id(stop_id_by_name_.at(stop->name));
    }
}

void Serializator::SaveDistances(const TransportCatalogue& catalogue) {
    auto &distances = catalogue.GetDistances();
    
    for (auto &[from_to, length] : distances) {
        proto_catalogue::Distance proto_distance;
        proto_distance.set_stop_id_from(stop_id_by_name_.at(from_to.first->name));
        proto_distance.set_stop_id_to(stop_id_by_name_.at(from_to.second->name));
        proto_distance.set_length(length);
        *proto_catalogue_.mutable_catalogue()->add_distance() = std::move(proto_distance);
    }
}

proto_catalogue::Coordinates
Serializator::MakeProtoCoordinates(const geo::Coordinates& coordinates) {
    proto_catalogue::Coordinates proto_coordinates;
    proto_coordinates.set_lat(coordinates.lat);
    proto_coordinates.set_lng(coordinates.lng);
    return proto_coordinates;
}

geo::Coordinates
Serializator::MakeCoordinates(const proto_catalogue::Coordinates& proto_coordinates) {
    geo::Coordinates coordinates;
    coordinates.lat = proto_coordinates.lat();
    coordinates.lng = proto_coordinates.lng();
    return coordinates;
}

void Serializator::LoadStops(TransportCatalogue& catalogue) {
    auto stops_count = proto_catalogue_.catalogue().stop_size();
    
    for (int i = 0; i < stops_count; ++i) {
        auto &proto_stop = proto_catalogue_.catalogue().stop(i);

        auto coords = MakeCoordinates(proto_stop.coordinates());

        catalogue.AddStop({proto_stop.name(), coords.lat, coords.lng});
        stop_name_by_id_.insert({proto_stop.id(), proto_stop.name()});
    }
}

void Serializator::LoadBuses(TransportCatalogue &catalogue) {
    auto buses_count = proto_catalogue_.catalogue().bus_size();
    
    for (int i = 0; i < buses_count; ++i) {
        auto& proto_bus = proto_catalogue_.catalogue().bus(i);
        LoadBus(catalogue, proto_bus);
        bus_name_by_id_.insert({proto_bus.id(), proto_bus.name()});
    }
}

void Serializator::LoadBus(TransportCatalogue& catalogue,
                             const proto_catalogue::Bus& proto_bus) const {
    auto stops_count = proto_bus.stop_id_size();
    
    std::vector<std::string> stops;
    
    stops.reserve(stops_count);
    
    for(int i = 0; i < stops_count; ++i) {
        auto stop_name = stop_name_by_id_.at(proto_bus.stop_id(i));
        stops.push_back(std::string(stop_name));
    }

    catalogue.AddBus({proto_bus.name(), move(stops), proto_bus.circular()});
}

void Serializator::LoadDistances(TransportCatalogue& catalogue) const {
    std::unordered_map<std::string, std::unordered_map<std::string, int>> d_map;

    auto distances_count = proto_catalogue_.catalogue().distance_size();
    
    for (int i = 0; i < distances_count; ++i) {
        auto &proto_distance = proto_catalogue_.catalogue().distance(i);
        
        auto stop_from = std::string(stop_name_by_id_.at(proto_distance.stop_id_from()));
        auto stop_to = std::string(stop_name_by_id_.at(proto_distance.stop_id_to()));

        d_map[stop_from][stop_to] = proto_distance.length();
    }

    for (auto& [from, mp_to] : d_map) {
        catalogue.AddDistances({move(from), move(mp_to)});
    }
}



} // serialize