#pragma once

#include "json.h"
#include "map_renderer.h"
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

private:
    json::Document input_;
    json::Node dummy_ = nullptr;

    std::pair<std::string_view, geo::Coordinates> PopulateStop(const json::Dict& request_map) const;
    void PopulateStopDistances(transport_catalogue::TransportCatalogue& catalogue) const;
    std::tuple<std::string_view, std::vector<std::string_view>, bool> PopulateBus(const json::Dict& request_map) const;
    std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba> ParseColor(const json::Node& color_node) const;
};