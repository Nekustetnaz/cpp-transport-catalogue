#include "transport_router.h"

namespace transport_router {

const std::optional<graph::Router<double>::RouteInfo> TransportRouter::GetRoute(
    const std::string_view stop_from
    , const std::string_view stop_to
    ) const {
	return router_->BuildRoute(stop_ids_.at(std::string(stop_from)),stop_ids_.at(std::string(stop_to)));
}

const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
	return graph_;
}

void TransportRouter::BuildGraph(const transport_catalogue::TransportCatalogue& catalogue) {
	const auto& all_stops = catalogue.GetAllStops();
	const auto& all_buses = catalogue.GetAllBuses();

	graph::DirectedWeightedGraph<double> stops_graph(all_stops.size() * 2);
    std::map<std::string, graph::VertexId> stop_ids;
    graph::VertexId vertex_id = 0;

    for (const auto& [stop_name, stop_info] : all_stops) {
        stop_ids[stop_info->name] = vertex_id;
        stops_graph.AddEdge({
                stop_info->name,
                0,
                vertex_id,
                vertex_id + 1,
                static_cast<double>(routing_settings_.bus_wait_time)
            });
        vertex_id += 2;
    }
    stop_ids_ = std::move(stop_ids);

    for (const auto& [bus_name, bus_info] : all_buses) {
        const auto& stops = bus_info->stops;
        const size_t stops_count = stops.size();

        for (size_t i = 0; i < stops_count; ++i) {
            for (size_t j = i + 1; j < stops_count; ++j) {
                const domain::Stop* stop_from = stops[i];
                const domain::Stop* stop_to = stops[j];

                int forward_distance = 0;
                int reverse_distance = 0; 
                for (size_t k = i + 1; k <= j; ++k) {
                    forward_distance += catalogue.GetDistance(stops[k - 1], stops[k]);
                    reverse_distance += catalogue.GetDistance(stops[k], stops[k - 1]);
                }

                const double velocity_factor = routing_settings_.bus_velocity * (100.0 / 6.0);
                const double travel_time = static_cast<double>(forward_distance) / velocity_factor;  

                stops_graph.AddEdge({
                    bus_info->name,
                    static_cast<size_t>(j - i),
                    stop_ids_.at(stop_from->name) + 1,
                    stop_ids_.at(stop_to->name),
                    travel_time
                });

                if (!bus_info->is_roundtrip) {
                    const double reverse_travel_time = static_cast<double>(reverse_distance) / velocity_factor;
                    stops_graph.AddEdge({
                        bus_info->name,
                        static_cast<size_t>(j - i),
                        stop_ids_.at(stop_to->name) + 1,
                        stop_ids_.at(stop_from->name),
                        reverse_travel_time
                    });
                }
            }
        }
    }

    graph_ = std::move(stops_graph);
    router_ = std::make_unique<graph::Router<double>>(graph_);
}

}