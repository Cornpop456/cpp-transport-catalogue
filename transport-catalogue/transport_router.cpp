#include "transport_router.h"

using namespace std;

route::TransportRouter::TransportRouter(const TransportCatalogue& catalogue,  RouteSettings route_setting) 
	: catalogue_(catalogue), route_setting_(move(route_setting)), graph_(catalogue.GetStopsSize()) {
	
	BuildGraph();
	router_ = make_unique<graph::Router<double>>(graph_);
}

optional<vector<route::RouteData>> route::TransportRouter::GetRoute(const string_view& from, const string_view& to) const {
	optional<graph::Router<double>::RouteInfo> route_info = router_->BuildRoute(catalogue_.GetStopId(from), catalogue_.GetStopId(to));

	if (route_info == nullopt) {
		return nullopt;
	}
	if (route_info.value().edges.size() == 0) {
		vector<RouteData> route_data;
		route_data.push_back(move(GetEmptyPart()));
		return route_data;
	}
	return BuildRoute(route_info);
}

void route::TransportRouter::BuildGraph() const {
	for (const auto bus_name : *catalogue_.GetBusNames()) {
		const Bus* struct_bus = catalogue_.GetBus(bus_name);

		AddEdges(struct_bus->bus_stops.begin(), struct_bus->bus_stops.end(), bus_name);
		if (!struct_bus->circular) {
			AddEdges(struct_bus->bus_stops.rbegin(), struct_bus->bus_stops.rend(), bus_name);
		}
	}
}

vector<route::RouteData> route::TransportRouter::BuildRoute(const optional<graph::Router<double>::RouteInfo>& route_info) const {
	vector<RouteData> route_data;

	for (size_t edge_index : route_info.value().edges) {
		route_data.push_back(move(GetWaitPart(edge_index)));
	
		double time = (edges_info_[edge_index].time_weight - route_setting_.bus_wait_time * 60) / 60;

		route_data.push_back(move(GetRidePart(edge_index, time)));
	}

	return route_data;
}

route::RouteData route::TransportRouter::GetRidePart(size_t edge_index, double time) const {
	RouteData bus_answer;
	bus_answer.type = "bus"sv;
	bus_answer.bus_name = edges_info_[edge_index].bus_name;
	bus_answer.span_count = edges_info_[edge_index].span_count;
	bus_answer.motion_time = time;
	return bus_answer;
}

route::RouteData route::TransportRouter::GetWaitPart(size_t edge_index) const {
	RouteData stop_answer;
	stop_answer.type = "stop"sv;
	stop_answer.stop_name = edges_[edge_index].first;
	stop_answer.bus_wait_time = route_setting_.bus_wait_time;
	return stop_answer;
}

route::RouteData route::TransportRouter::GetEmptyPart() const {
	RouteData empty_answer;
	empty_answer.type = "stay_here"sv;
	return empty_answer;
}