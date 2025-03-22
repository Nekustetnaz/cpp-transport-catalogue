#pragma once

#include "json.h"
#include "json_reader.h"
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

    void ProcessRequests(const json::Node& stat_requests) const;
    
    const json::Node PrintBus(const json::Dict& request_map) const;
    const json::Node PrintStop(const json::Dict& request_map) const;
    const json::Node PrintMap(const json::Dict& request_map) const;
 
    svg::Document RenderMap() const;

private:
    const map_renderer::MapRenderer& renderer_;
    const transport_catalogue::TransportCatalogue& catalogue_;
};