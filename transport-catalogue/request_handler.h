#pragma once

#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"


class RequestHandler {
public:
    explicit RequestHandler(const map_renderer::MapRenderer& renderer, transport_catalogue::TransportCatalogue& catalogue)
        : renderer_(renderer)
        , catalogue_(catalogue)
    {
    }

    bool IsBusExists(std::string_view bus_name) const;
    bool IsStopExists(std::string_view stop_name) const;

    const domain::RouteInfo GetRoute(std::string_view bus_name) const;
    const std::vector<std::string_view> GetBuses(std::string_view stop_name) const;
 
    svg::Document RenderMap() const;

private:
    const map_renderer::MapRenderer& renderer_;
    const transport_catalogue::TransportCatalogue& catalogue_;
};