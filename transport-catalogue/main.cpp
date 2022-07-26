#include <iostream>

#include "json_reader.h"
#include "transport_router.h"

using namespace std;

int main() {
    using namespace transport;
    using namespace route;
    
    TransportCatalogue catalogue;
    JsonReader reader(cin);

    reader.FillCatalogue(catalogue);

    renderer::MapRenderer renderer = reader.GetRenderer(catalogue);

    auto router = TransportRouter(catalogue, reader.GetRouteSettings());

    RequestHandler handler(catalogue, router, renderer);

    reader.PrintJsonResponse(handler, cout);
}

