#pragma once

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

#include <string>
#include <unordered_map>
#include <set>
#include <vector>

#include "geo.h"

namespace transport {

struct BusStat {
    int all_stops;
    int unique_stops;
    unsigned int length;
    double curvature;
};

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
    std::set<std::string_view> buses_through;
    int id;
};

struct Bus {
    std::string name;
    std::vector<Stop*> bus_stops;
    bool circular;
};

namespace parsed {

struct Bus {
    std::string name;
    std::vector<std::string> stops;
    bool circular;
};

struct Distances {
    std::string from;
    std::unordered_map<std::string, int> d_map;
};

struct Stop {
    std::string name;
    double lat;
    double lng;
};

} // parsed

} // transport