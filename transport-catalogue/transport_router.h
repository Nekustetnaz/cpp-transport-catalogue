#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <map>
#include <memory>

namespace transport_router {

struct RoutingSettings {
	int bus_wait_time = 0;
	double bus_velocity = 0.0;
};

class TransportRouter {
public:
	TransportRouter() = default;

	TransportRouter(const RoutingSettings& routing_settings, const transport_catalogue::TransportCatalogue& catalogue)
		: routing_settings_(routing_settings)
        {
			BuildGraph(catalogue);
		}

	const std::optional<graph::Router<double>::RouteInfo> GetRoute(
        std::string_view stop_from
        , std::string_view stop_to
        ) const;

	const graph::DirectedWeightedGraph<double>& GetGraph() const;

private:
    const RoutingSettings routing_settings_;

	graph::DirectedWeightedGraph<double> graph_;
	std::map<std::string, graph::VertexId> stop_ids_;
	std::unique_ptr<graph::Router<double>> router_;

	void BuildGraph(const transport_catalogue::TransportCatalogue& catalogue);
};

}