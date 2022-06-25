#include <iostream>

#include "json_reader.h"

using namespace std;

int main() {
    using namespace transport;
    
    TransportCatalogue catalogue;
    JsonReader reader(cin);

    reader.FillCatalogue(catalogue);

    renderer::MapRenderer renderer = reader.GetRenderer(catalogue);

    RequestHandler handler(catalogue, renderer);

    reader.PrintJsonResponse(handler, cout);
}

