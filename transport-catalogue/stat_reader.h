#pragma once 

#include <iostream>

#include "transport_catalogue.h"

namespace transport {

namespace output {

enum class QueryType {
    STOP,
    BUS,
    WRONG
};

std::pair<std::string, QueryType> ReadQuery();

void PrintBusStat(const std::string& name, 
    const std::optional<TransportCatalogue::BusStat>& opt_stat, 
    std::ostream& out);

void PrintBusesThroughStop(const std::string& name, 
    const std::optional<std::set<std::string_view>> opt_buses,
    std::ostream& out);

std::ostream& operator<<(std::ostream& os, const TransportCatalogue::BusStat& stat);    

} // output

} // transport
