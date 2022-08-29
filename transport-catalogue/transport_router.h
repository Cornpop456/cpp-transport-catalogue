#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>


namespace route {

using namespace transport;
using namespace std::literals;

struct RouteWeight {
	std::string_view bus_name;
	double total_time = 0;
	int span_count = 0;
};

struct RouteSettings {
	int bus_wait_time = 0;
	int bus_velocity = 0;
};

bool operator<(const RouteWeight& left, const RouteWeight& right);
bool operator>(const RouteWeight& left, const RouteWeight& right);
RouteWeight operator+(const RouteWeight& left, const RouteWeight& right);

class TransportRouter {
public:

    using Graph = graph::DirectedWeightedGraph<RouteWeight>;
    using Router = graph::Router<RouteWeight>;

    struct RouterEdge {
        std::string bus_name;
        std::string stop_from;
        std::string stop_to;
        double total_time = 0;
        int span_count = 0;
    };
    using TransportRoute = std::vector<RouterEdge>;

    TransportRouter(const transport::TransportCatalogue& catalogue,
        const RouteSettings& settings);

    std::optional<TransportRoute> BuildRoute(const std::string& from, const std::string& to);

    const RouteSettings& GetSettings() const;
    RouteSettings& GetSettings();

    void InitRouter();
    void InternalInit();

    Graph& GetGraph();
    const Graph& GetGraph() const;

    std::unique_ptr<Router>& GetRouter();
    const std::unique_ptr<Router>& GetRouter() const;
private:

    bool is_initialized_ = false;

    const transport::TransportCatalogue &catalogue_;
    RouteSettings settings_;

    Graph graph_;
    mutable std::unique_ptr<Router> router_;

    void BuildEdges();
    graph::Edge<RouteWeight> BuildEdge(const transport::Bus* bus, int stop_from_index, int stop_to_index);
    double ComputeTime(const transport::Bus* bus, int stop_from_index, int stop_to_index);
};

} 