#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace transport {
 
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

namespace input {

enum class QueryType {
    STOP,
    BUS,
    WRONG
};

std::pair<std::string, QueryType> ReadQuery();

std::pair<parsed::Stop, parsed::Distances> ReadStop(std::string_view str);

parsed::Bus ReadBus(std::string_view str);

} // input 

namespace detail {

int ReadNumber();

} // detail


} // transport