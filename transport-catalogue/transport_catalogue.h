#pragma once

#include <deque>
#include <optional>
#include <set>

#include "domain.h"

namespace transport {

class TransportCatalogue {
private:
    struct DistanceHasher {
        size_t operator()(const std::pair<Stop*, Stop*>& p) const;

    private:
        std::hash<double> d_hasher_;
    };
    

    int stop_id_ = 0;
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    
    std::deque<Bus> buses_;
    std::set<std::string_view> bus_names_;
    std::unordered_map<std::string_view, BusStat> bus_stats_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;

    std::unordered_map<int, Stop*> stop_id_to_stop_;

    std::unordered_map<std::pair<Stop*, Stop*>, int, DistanceHasher> distances_;

    BusStat CalculateStat(const std::string& name) const;
    
public:
    void AddStop(const parsed::Stop& stop);
    void AddBus(const parsed::Bus& route);
    void AddDistances(const parsed::Distances& dists);
    
    bool FindStop(const std::string& name) const;
    bool FindBus(const std::string& name) const;

    int GetStopsSize() const;
    int GetStopId(const std::string_view& name) const;
    std::string GetStopNameById(int id) const;

    const std::unordered_map<std::string_view, Stop*> GetStops() const;
    const std::unordered_map<std::string_view, Bus*> GetBuses() const;
    const std::unordered_map<std::pair<Stop*, Stop*>, int, DistanceHasher> GetDistances() const;

    const std::set<std::string_view>* GetBusNames() const;
    const Bus* GetBus(std::string_view name) const;
    std::set<std::string_view>* GetBusesThroughStop(const std::string& name) const;
    std::optional<BusStat> GetBusStat(const std::string& name) const;
    unsigned int GetStopsDistance(const std::string& from, const std::string& dest) const;
};

} // transport