#include <iostream>
#include <memory>
#include <fstream>
#include <string_view>

#include "json_reader.h"
#include "transport_router.h"

using namespace std::literals;
using namespace std;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int prev_main() {
    using namespace transport;
    using namespace route;
    
    TransportCatalogue catalogue;
    JsonReader reader(cin);

    reader.FillCatalogue(catalogue);

    RequestHandler handler(catalogue);

    handler.SetTransportRouter(make_unique<TransportRouter>(catalogue, reader.GetRouteSettings()));
    handler.SetRenderer(make_unique<renderer::MapRenderer>(reader.GetRenderer(catalogue)));

    reader.PrintJsonResponse(handler, cout);

    return 0;
}

int main(int argc, char* argv[]) {
    using namespace transport;
    using namespace route;

    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    TransportCatalogue catalogue;

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        JsonReader reader(cin);
        reader.FillCatalogue(catalogue);
        RequestHandler handler(catalogue);

        handler.Serialize(reader.GetSerializeSettings());

        cout << reader.GetSerializeSettings().file << endl;

    } else if (mode == "process_requests"sv) {

        // process requests here

    } else {
        PrintUsage();
        return 1;
    }

    return 0;
}