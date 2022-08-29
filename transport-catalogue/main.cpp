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

// int prev_main() {
//     using namespace transport;
//     using namespace route;
    
//     TransportCatalogue catalogue;
//     JsonReader reader(cin);

//     reader.FillCatalogue(catalogue);

//     RequestHandler handler(catalogue);

//     handler.SetTransportRouter(make_unique<TransportRouter>(catalogue, reader.GetRouteSettings()));

//     reader.PrintJsonResponse(handler, cout);

//     return 0;
// }

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

        handler.Serialize(reader.GetSerializeSettings(), reader.GetRenderSettings(), reader.GetRouteSettingsOpt());

    } else if (mode == "process_requests"sv) {
        JsonReader reader(cin);
        RequestHandler handler(catalogue);

        handler.Deserialize(reader.GetSerializeSettings());

        reader.PrintJsonResponse(handler, cout);

        // ofstream svg("out.svg");

        // handler.RenderMap().Render(svg);

    } else {
        PrintUsage();
        return 1;
    }

    return 0;
}