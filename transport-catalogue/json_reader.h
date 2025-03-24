#pragma once

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

#include <iostream>

class JsonReader {
public:
    JsonReader(std::istream& input)
        : input_(json::Load(input))
    {}

    const json::Node& GetBaseRequests() const;
    const json::Node& GetStatRequests() const;
    const json::Node& GetRenderSettings() const;

    void PopulateCatalogue(transport_catalogue::TransportCatalogue& catalogue);
    map_renderer::MapRenderer FillRenderSettings(const json::Dict& request_map) const;

    void ProcessStatRequests(const json::Node& stat_requests, const RequestHandler& handler) const;
    const json::Node PrintBus(const json::Dict& request_map, const RequestHandler& handler) const;
    const json::Node PrintStop(const json::Dict& request_map, const RequestHandler& handler) const;
    const json::Node PrintMap(const json::Dict& request_map, const RequestHandler& handler) const;

private:
    json::Document input_;
    json::Node dummy_ = nullptr;

    std::pair<std::string_view, geo::Coordinates> ParseStop(const json::Dict& request_map) const;
    void PopulateStop(const json::Array& base_requests_arr, transport_catalogue::TransportCatalogue& catalogue) const;

    void PopulateStopDistances(const json::Array& base_requests_arr, transport_catalogue::TransportCatalogue& catalogue) const;

    std::tuple<std::string_view, std::vector<std::string_view>, bool> ParseBus(const json::Dict& request_map) const;
    void PopulateBus(const json::Array& base_requests_arr, transport_catalogue::TransportCatalogue& catalogue) const;

    std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba> ParseColor(const json::Node& color_node) const;
};