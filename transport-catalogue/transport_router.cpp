#include "transport_router.h"

#include <iostream>

namespace route {

TransportRouter::TransportRouter(const transport::TransportCatalogue& catalogue,
    const RouteSettings& settings) : catalogue_(catalogue), settings_(settings) {
}

void TransportRouter::InitRouter() {
    if (!is_initialized_) {
        graph::DirectedWeightedGraph<RouteWeight>graph(catalogue_.GetStopsSize());
        graph_ = std::move(graph);
 
        BuildEdges();

        router_ = std::make_unique<graph::Router<RouteWeight>>(graph_);
        is_initialized_ = true;
    }
}

std::optional<TransportRouter::TransportRoute>
TransportRouter::BuildRoute(const std::string& from, const std::string& to) {
    if (from == to) {
        return TransportRoute{};
    }

    InitRouter();

    auto from_id = catalogue_.GetStopId(from);
    auto to_id = catalogue_.GetStopId(to);
    auto route = router_->BuildRoute(from_id, to_id);
    
    if (!route) {
        return std::nullopt;
    }

    TransportRoute result;

    for (auto edge_id : route->edges) {
        const auto &edge = graph_.GetEdge(edge_id);
        RouterEdge route_edge;
        route_edge.bus_name = edge.weight.bus_name;
        route_edge.stop_from = catalogue_.GetStopNameById(edge.from);
        route_edge.stop_to = catalogue_.GetStopNameById(edge.to);
        route_edge.span_count = edge.weight.span_count;
        route_edge.total_time = edge.weight.total_time;

        result.push_back(route_edge);
    }
    return result;
}

const RouteSettings& TransportRouter::GetSettings() const {
    return settings_;
}

RouteSettings& TransportRouter::GetSettings() {
    return settings_;
}

void TransportRouter::InternalInit() {
    is_initialized_ = true;
}

TransportRouter::Graph& TransportRouter::GetGraph() {
    return graph_;
}
const TransportRouter::Graph& TransportRouter::GetGraph() const {
    return graph_;
}

std::unique_ptr<TransportRouter::Router>& TransportRouter::GetRouter() {
    return router_;
}
const std::unique_ptr<TransportRouter::Router>& TransportRouter::GetRouter() const {
    return router_;
}


void TransportRouter::BuildEdges() {
    for (const auto& [bus_name, bus] : catalogue_.GetBuses()) {
        int stops_count = static_cast<int>(bus->bus_stops.size());

        for(int i = 0; i < stops_count - 1; ++i) {

            double route_time = settings_.bus_wait_time;
            double route_time_back = settings_.bus_wait_time;

            for(int j = i + 1; j < stops_count; ++j) {
                graph::Edge<RouteWeight> edge = BuildEdge(bus, i, j);
                route_time += ComputeTime(bus, j - 1, j);
                edge.weight.total_time = route_time;
                graph_.AddEdge(edge);

                if (!bus->circular) {
                    int i_back = stops_count - 1 - i;
                    int j_back = stops_count - 1 - j;
                    
                    graph::Edge<RouteWeight> edge = BuildEdge(bus, i_back, j_back);
                    
                    route_time_back += ComputeTime(bus, j_back + 1, j_back);
                    edge.weight.total_time = route_time_back;
                    graph_.AddEdge(edge);
                }
            }
        }
    }
}

graph::Edge<RouteWeight> TransportRouter::BuildEdge(const transport::Bus* bus,
    int stop_from_index, int stop_to_index) {

    graph::Edge<RouteWeight> edge;
    
    edge.from = catalogue_.GetStopId(bus->bus_stops.at(static_cast<size_t>(stop_from_index))->name);
    edge.to = catalogue_.GetStopId(bus->bus_stops.at(static_cast<size_t>(stop_to_index))->name);
    
    edge.weight.bus_name = bus->name;
    edge.weight.span_count = static_cast<int>(stop_to_index - stop_from_index);
    
    return edge;
}

double TransportRouter::ComputeTime(const transport::Bus* bus, int stop_from_index, int stop_to_index) {
    auto distance = catalogue_.GetStopsDistance(bus->bus_stops.at(static_cast<size_t>(stop_from_index))->name,
        bus->bus_stops.at(static_cast<size_t>(stop_to_index))->name);
    
    return distance / (settings_.bus_velocity * 1000.0 / 60.0);
}

bool operator<(const RouteWeight& left, const RouteWeight& right) {
    return left.total_time < right.total_time;
}

RouteWeight operator+(const RouteWeight& left, const RouteWeight& right) {
    RouteWeight result;
    result.total_time = left.total_time + right.total_time;
    return result;
}

bool operator>(const RouteWeight& left, const RouteWeight& right) {
    return left.total_time > right.total_time;
}

} // namespace transport_router