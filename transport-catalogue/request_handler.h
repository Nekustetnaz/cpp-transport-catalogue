#pragma once

#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"


class RequestHandler {
public:
    explicit RequestHandler(
        const map_renderer::MapRenderer& renderer
        , const transport_catalogue::TransportCatalogue& catalogue
        , const transport_router::TransportRouter& router
        )
        : renderer_(renderer)
        , catalogue_(catalogue)
        , router_(router)
    {
    }

    bool IsBusExists(std::string_view bus_name) const;
    bool IsStopExists(std::string_view stop_name) const;

    const domain::RouteInfo GetRouteInfo(std::string_view bus_name) const;
    const std::vector<std::string_view> GetBuses(std::string_view stop_name) const;

    const std::optional<graph::Router<double>::RouteInfo> GetBestRoute(
        std::string_view stop_from
        , std::string_view stop_to
        ) const;
    const graph::DirectedWeightedGraph<double>& GetRouteGraph() const;
 
    svg::Document RenderMap() const;

private:
    const map_renderer::MapRenderer& renderer_;
    const transport_catalogue::TransportCatalogue& catalogue_;
    const transport_router::TransportRouter& router_;
};