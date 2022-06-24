#pragma once

#include <iostream>

#include "map_renderer.h"
#include "transport_catalogue.h"

namespace transport {

enum class Format {
    SVG,
    JSON
};

namespace json_reader {

void ProcessJSON(TransportCatalogue& catalogue, std::istream& in, std::ostream& out, 
    Format out_format = Format::JSON);

} // json_reader

} // transport