#include <iostream>

#include "catalogue_client.h"
#include "log_duration.h"

using namespace std;
using namespace transport;

int main() {    
    TransportCatalogue catalogue;
    {
    LOG_DURATION_STREAM("fill"s, cerr);
    client::FillCatalogue(catalogue, cin);
    }

    {
    LOG_DURATION_STREAM("read"s, cerr);
    client::ProcessQueries(catalogue, cin, cout);
    }

    return 0;
}