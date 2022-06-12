#include <iostream>

#include "catalogue_client.h"

using namespace std;
using namespace transport;

int main() {    
    TransportCatalogue catalogue;
    client::FillCatalogue(catalogue, cin);

    client::ProcessQueries(catalogue, cin, cout);

    return 0;
}