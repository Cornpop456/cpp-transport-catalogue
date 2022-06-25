#include <iostream>

#include "json_reader.h"

using namespace std;

int main() {
    using namespace transport;
    
    TransportCatalogue catalogue;
    json_reader::ProcessJSON(catalogue, cin, cout, Format::JSON);
}

