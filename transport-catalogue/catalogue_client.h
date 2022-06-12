#pragma once

#include "transport_catalogue.h"

namespace transport {

namespace client {

void FillCatalogue(TransportCatalogue& catalogue, std::istream& in);

void ProcessQueries(TransportCatalogue& catalogue, std::istream& in, std::ostream& out);

} // client

} // transport