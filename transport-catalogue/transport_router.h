#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <map>
#include <memory>

namespace transport_router {

struct RoutingSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0.0;

    static constexpr double KMH_TO_METERS_PER_MIN = 1000.0 / 60.0;
};

class TransportRouter {
public:
    TransportRouter() = default;

    TransportRouter(const RoutingSettings& routing_settings, const transport_catalogue::TransportCatalogue& catalogue)
	: routing_settings_(routing_settings)
	, catalogue_(catalogue)
        {
	   BuildGraph();
	}

    const std::optional<std::vector<const graph::Edge<double>*>> GetRoute(
    std::string_view stop_from, std::string_view stop_to) const;

private:
    const RoutingSettings routing_settings_;
    const transport_catalogue::TransportCatalogue& catalogue_;

    graph::DirectedWeightedGraph<double> graph_;
    std::map<std::string, graph::VertexId> stop_ids_;
    std::unique_ptr<graph::Router<double>> router_;

    void ProcessAllStops(graph::DirectedWeightedGraph<double>& stops_graph, std::map<std::string, graph::VertexId>& stop_ids);
    void ProcessAllBuses(graph::DirectedWeightedGraph<double>& stops_graph);
    void BuildGraph();
};

}
