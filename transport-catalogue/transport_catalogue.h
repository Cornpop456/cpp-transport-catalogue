#pragma once

#include <deque>
#include <optional>
#include <set>

#include "domain.h"

namespace transport {

class TransportCatalogue {
public:
    struct BusStat {
        int all_stops;
        int unique_stops;
        unsigned int length;
        double curvature;
    };

private:
    struct DistanceHasher {
        size_t operator()(const std::pair<Stop*, Stop*>& p) const;

    private:
        std::hash<double> d_hasher_;
    };
    
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, BusStat> bus_stats_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;

    std::unordered_map<std::pair<Stop*, Stop*>, int, DistanceHasher> distances_;

    BusStat CalculateStat(const std::string& name) const;
    
public:
    void AddStop(const parsed::Stop& stop);
    void AddBus(const parsed::Bus& route);
    void AddDistances(const parsed::Distances& dists);
    
    bool FindStop(const std::string& name) const;
    bool FindBus(const std::string& name) const;

    std::set<std::string_view>* GetBusesThroughStop(const std::string& name) const;
    std::optional<BusStat> GetBusStat(const std::string& name) const;
    unsigned int GetStopsDistance(const std::string& from, const std::string& dest) const;
};

} // transport