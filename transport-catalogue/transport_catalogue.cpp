#include "transport_catalogue.h"

#include <iostream>

using namespace std;

namespace transport {

int TransportCatalogue::GetStopsSize() const {
    return stopname_to_stop_.size();
}

int TransportCatalogue::GetStopId(const std::string_view& name) const {
    return stopname_to_stop_.at(name)->id;
}

void TransportCatalogue::AddStop(const parsed::Stop& stop) {
    int tmp_id = stop_id_;

    Stop s = Stop{stop.name, geo::Coordinates{stop.lat, stop.lng}, set<string_view>{}, stop_id_++};
    
    stops_.push_back(move(s));

    Stop* added = &stops_.back();

    stopname_to_stop_[string_view{added->name}] = added;
    stop_id_to_stop_[tmp_id] = added;
}

void TransportCatalogue::AddDistances(const parsed::Distances& dists) {
    Stop* from = stopname_to_stop_.at(dists.from);

    for (const auto& [dest, meters] : dists.d_map) {
        Stop* to = stopname_to_stop_.at(dest);

        distances_[{from, to}] = meters;

        if (distances_.count({to, from}) == 0) {
            distances_[{to, from}] = meters;
        }
    }
}

void TransportCatalogue::AddBus(const parsed::Bus& bus) {
    Bus b;
    b.name = bus.name;
    b.circular = bus.circular;

    buses_.push_back(move(b));

    Bus* added = &buses_.back();
    bus_names_.insert(string_view{added->name});

    for (const string& el : bus.stops) {
        added->bus_stops.push_back(stopname_to_stop_.at(el));
        stopname_to_stop_.at(el)->buses_through.insert(string_view{added->name});
    }

    busname_to_bus_[string_view{added->name}] = added;

    bus_stats_[string_view{added->name}] = CalculateStat(added->name);
}

bool TransportCatalogue::FindStop(const string& name) const {
    return stopname_to_stop_.count(name) > 0;
}

bool TransportCatalogue::FindBus(const string& name) const {
    return busname_to_bus_.count(name) > 0;
}

optional<BusStat> TransportCatalogue::GetBusStat(const string& name) const {
    if (bus_stats_.count(name) == 0) {
        return nullopt;
    }

    return bus_stats_.at(name);
}

std::string TransportCatalogue::GetStopNameById(int id) const {
    return stop_id_to_stop_.at(id)->name;
}

set<string_view>* TransportCatalogue::GetBusesThroughStop(const string& name) const {
    if (stopname_to_stop_.count(name) == 0) {
        return nullptr;
    }

    return &stopname_to_stop_.at(name)->buses_through;
}

unsigned int TransportCatalogue::GetStopsDistance(const string& from, const string& dest) const {
    return distances_.at({stopname_to_stop_.at(from), stopname_to_stop_.at(dest)});
}


const set<string_view>* TransportCatalogue::GetBusNames() const {
    return &bus_names_;
}

const Bus* TransportCatalogue::GetBus(string_view name) const {
    return busname_to_bus_.at(name);
}

const std::unordered_map<std::string_view, Stop*> TransportCatalogue::GetStops() const {
    return stopname_to_stop_;
}

const std::unordered_map<std::string_view, Bus*> TransportCatalogue::GetBuses() const {
    return busname_to_bus_;
}

const std::unordered_map<std::pair<Stop*, Stop*>, int, TransportCatalogue::DistanceHasher> TransportCatalogue::GetDistances() const {
    return distances_;
}

BusStat TransportCatalogue::CalculateStat(const string& name) const {
    Bus* b = busname_to_bus_.at(name);

    int stops_count = b->bus_stops.size();
    int unique_stops_count = 0;
    double geo_length = 0;
    unsigned int actual_length = 0;

    set<string_view> uniq_stops;

    for (size_t i = 0; i < b->bus_stops.size(); ++i) {
        if (i == b->bus_stops.size() - 1) {
            if (uniq_stops.count(b->bus_stops[i]->name) == 0) {
                ++unique_stops_count;
            }

            break;
        }

        if (uniq_stops.count(b->bus_stops[i]->name) == 0) {
            ++unique_stops_count;
            uniq_stops.insert(b->bus_stops[i]->name);
        }

        geo_length += geo::ComputeDistance(b->bus_stops[i]->coordinates, b->bus_stops[i + 1]->coordinates);
        actual_length += distances_.at({b->bus_stops[i], b->bus_stops[i + 1]});
    }

    if (!b->circular) {
        geo_length *= 2;
        stops_count *=2;
        --stops_count;

        for (size_t i = b->bus_stops.size() - 1; i > 0; --i) {
            actual_length += distances_.at({b->bus_stops[i], b->bus_stops[i - 1]});
        }
    }

    return BusStat{stops_count, unique_stops_count, actual_length, actual_length / geo_length};
}

size_t TransportCatalogue::DistanceHasher::operator()(const pair<Stop*, Stop*>& p) const {
    size_t lat_1 = d_hasher_(p.first->coordinates.lat);
    size_t lng_1 = d_hasher_(p.first->coordinates.lng);

    size_t lat_2 = d_hasher_(p.second->coordinates.lat);
    size_t lng_2 = d_hasher_(p.second->coordinates.lng);

    return lat_1 + lng_1 * 37 + lat_2 * (37 * 37) + lng_2 * (37 * 37 * 37);
}

} // transport