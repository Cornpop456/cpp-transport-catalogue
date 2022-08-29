cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

set (proto "transport_catalogue.proto" "svg.proto" "map_renderer.proto" "graph.proto" "transport_router.proto")

set (sources
    "main.cpp"
    "domain.cpp"
    "geo.cpp"
    "json.cpp"
    "json_builder.cpp"
    "json_reader.cpp"
    "map_renderer.cpp"
    "request_handler.cpp"
    "serialization.cpp"
    "svg.cpp"
    "transport_catalogue.cpp"
    "transport_router.cpp"
    )

set (headers
    "domain.h"
    "geo.h"
    "graph.h"
    "json.h"
    "json_builder.h"
    "json_reader.h"
    "map_renderer.h"
    "ranges.h"
    "request_handler.h"
    "router.h"
    "serialization.h"
    "svg.h"
    "transport_catalogue.h"
    "transport_router.h"
   )

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${proto})

add_executable(transport_catalogue ${sources} ${headers} ${proto} ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(transport_catalogue PRIVATE "include")

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY_RELEASE}>" Threads::Threads)
