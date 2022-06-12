#include "stat_reader.h"

#include <iomanip>

using namespace std;

namespace transport {

pair<string, output::QueryType> output::ReadQuery(std::istream& in) {
    string query_name;

    in >> query_name;
    
    QueryType t;
    
    if (query_name == "Stop"s) {
        t = QueryType::STOP;
    } else if (query_name == "Bus"s) {
        t = QueryType::BUS;
    } else {
        t = QueryType::WRONG;
    }

    string str; 
    getline(in, str);

    if (str.empty()) {
        return {"", t};
    }

    return {str.substr(1), t};
}

void output::PrintBusStat(const string& name, 
    const optional<TransportCatalogue::BusStat>& opt_stat, 
    ostream& out) { // <- вывод уже был вынесен в аргумент out

    if (opt_stat) {
        out << "Bus " << name << ": " << opt_stat.value() << endl;
        return;
    }

    out << "Bus " << name << ": " << "not found" << endl;
}

void output::PrintBusesThroughStop(const std::string& name, 
    const optional<set<string_view>> opt_buses,
    std::ostream& out) { // <- вывод уже был вынесен в аргумент out

    if (!opt_buses) {
        out << "Stop " << name << ": " << "not found" << endl;
        return;
    }

    const auto& buses = opt_buses.value();

    if (buses.size() == 0) {
        out << "Stop " << name << ": " << "no buses" << endl;
        return;
    }

    out << "Stop " << name << ": buses ";
    
    int index = 0;

    for (auto el : buses) {
        if (index == buses.size() - 1) {
            out << el;
            break;
        }

        out << el << " ";
        ++index;
    }

    out << endl;
}

ostream& output::operator<<(ostream& os, const TransportCatalogue::BusStat& stat) {
    os  << stat.all_stops 
        << " stops on route, " 
        << stat.unique_stops
        << " unique stops, "
        << stat.length
        << " route length, "
        << setprecision(6)
        << stat.curvature 
        << " curvature";

    return os;
}

} // transport
