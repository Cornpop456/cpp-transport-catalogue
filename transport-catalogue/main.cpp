#include <stdexcept>
#include <vector>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

using namespace std;

using namespace transport;

int main() {
    TransportCatalogue catalogue;

	int input_n = detail::ReadNumber();

    vector<string> add_bus_deferred;
    vector<parsed::Distances> add_dists_deferred;

    for (int i = 0; i < input_n; ++i) {
        auto [s, type] =  input::ReadQuery();

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

    for (const string& s : add_bus_deferred) {
        auto parsed_bus = input::ReadBus(s);
        catalogue.AddBus(parsed_bus);
    }

    add_bus_deferred.clear();

    for (const auto& d : add_dists_deferred) {
        catalogue.AddDistances(d);
    }

    add_dists_deferred.clear();

    int output_n = detail::ReadNumber();

    for (int i = 0; i < output_n; ++i) {
        auto [s, type] = output::ReadQuery();

        if (s.empty() || type == output::QueryType::WRONG) {
            throw invalid_argument("wrong query to catalogue"s);
        }

        if (type == output::QueryType::STOP) {
            output::PrintBusesThroughStop(s, catalogue.GetBusesThroughStop(s), cout);
        } else if (type == output::QueryType::BUS) {
            output::PrintBusStat(s, catalogue.GetBusStat(s), cout);
        }
    }

    return 0;
}