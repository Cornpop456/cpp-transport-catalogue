#pragma once

#include <iostream>

#include "transport_catalogue.h"

namespace transport {

namespace json_reader {

void ProcessJSON(TransportCatalogue& catalogue, std::istream& in, std::ostream& out);

} // json_reader

} // transport