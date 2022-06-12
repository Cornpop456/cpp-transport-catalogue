#include "catalogue_client.h"
#include "stat_reader.h"

using namespace std;

namespace transport {

void client::FillCatalogue(TransportCatalogue& catalogue, istream& in) {
    int input_n = detail::ReadNumber(in);

    vector<string> add_bus_deferred;
    vector<parsed::Distances> add_dists_deferred;

    for (int i = 0; i < input_n; ++i) {
        auto [s, type] =  input::ReadQuery(in);

        if (s.empty() || type == input::QueryType::WRONG) {
            throw invalid_argument("wrong query to catalogue"s);
        }

        if (type == input::QueryType::STOP) {
            auto [parsed_stop, parsed_distances] = input::ReadStop(s);

            if (parsed_distances.d_map.size() > 0) {
                add_dists_deferred.push_back(move(parsed_distances));
            }

            catalogue.AddStop(parsed_stop);
        } else if (type == input::QueryType::BUS) {
            add_bus_deferred.push_back(s);
        }
    }

    for (const auto& d : add_dists_deferred) {
        catalogue.AddDistances(d);
    }

    for (const string& s : add_bus_deferred) {
        auto parsed_bus = input::ReadBus(s);
        catalogue.AddBus(parsed_bus);
    }
}

void client::ProcessQueries(TransportCatalogue& catalogue, istream& in, ostream& out) {
    int output_n = detail::ReadNumber(in);

    for (int i = 0; i < output_n; ++i) {
        auto [s, type] = output::ReadQuery(in);

        if (s.empty() || type == output::QueryType::WRONG) {
            throw invalid_argument("wrong query to catalogue"s);
        }

        if (type == output::QueryType::STOP) {
            output::PrintBusesThroughStop(s, catalogue.GetBusesThroughStop(s), out);
        } else if (type == output::QueryType::BUS) {
            output::PrintBusStat(s, catalogue.GetBusStat(s), out);
        }
    }
}

} // transport