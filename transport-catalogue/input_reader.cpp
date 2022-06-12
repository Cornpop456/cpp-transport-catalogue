#include "input_reader.h"

#include <iostream>

using namespace std;

namespace transport {

int detail::ReadNumber(istream& in) {
    int res;

    in >> res;

    string line_feed; 
    getline(in, line_feed);

    return res;
}

pair<string, input::QueryType> input::ReadQuery(istream& in) {
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

pair<parsed::Stop, parsed::Distances> input::ReadStop(string_view str) {
    parsed::Stop p_s;
    parsed::Distances p_d;

    size_t colon = str.find(':');
    p_s.name = string(str.substr(0, colon));
    p_d.from = p_s.name;
    str.remove_prefix(colon+2);

    size_t comma = str.find(',');
    p_s.lat = stod(string(str.substr(0, comma)));
    str.remove_prefix(comma+2);

    comma = str.find(',');
    p_s.lng = stod(string(str.substr(0, comma)));

    if (comma == str.npos) {
        return {p_s, p_d};  
    } else {
        str.remove_prefix(comma+2);
    }

    while (true) {
        comma = str.find(',');
        string_view next_dist = str.substr(0, comma);

        if (!next_dist.empty()) {
            int meters;
            string dest;

            size_t m = str.find('m');
            meters = stoi(string(next_dist.substr(0, m)));
            
            next_dist.remove_prefix(m + 5); // delete prefix {} {number+m to }dest
            
            p_d.d_map[string(next_dist)] = meters;
        }

        if (comma == str.npos) {
            break;
        } else {
            str.remove_prefix(comma+2);
        }
    }

    return {p_s, p_d};
}

parsed::Bus input::ReadBus(string_view str) {
  parsed::Bus bus;

  size_t colon = str.find(':');
  bus.name = string(str.substr(0, colon));
  str.remove_prefix(colon+2);

  size_t dash = str.find('-');

  if (dash != str.npos) {
    bus.circular = false;
  } else {
    bus.circular = true;
  }

  char split_symbol = bus.circular ? '>' : '-';

  while (true) {
        size_t symbol = str.find(split_symbol);
        string_view s = str.substr(0, symbol - 1);
        if (!s.empty()) {
            bus.stops.push_back(string(s));
        }
        if (symbol == str.npos) {
            break;
        } else {
            str.remove_prefix(symbol+2);
        }
    }

    return bus;
}

} // transport 