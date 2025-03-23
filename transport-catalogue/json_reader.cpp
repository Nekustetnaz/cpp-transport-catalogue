#include "json_reader.h"

using namespace std;

const json::Node& JsonReader::GetBaseRequests() const {
    if (!input_.GetRoot().AsMap().count("base_requests")) return dummy_;
    return input_.GetRoot().AsMap().at("base_requests");
}

const json::Node& JsonReader::GetStatRequests() const {
    if (!input_.GetRoot().AsMap().count("stat_requests")) return dummy_;
    return input_.GetRoot().AsMap().at("stat_requests");
}

const json::Node& JsonReader::GetRenderSettings() const {
    if (!input_.GetRoot().AsMap().count("render_settings")) return dummy_;
    return input_.GetRoot().AsMap().at("render_settings");
}

void JsonReader::PopulateCatalogue(transport_catalogue::TransportCatalogue& catalogue) {
    const json::Array& base_requests_arr = GetBaseRequests().AsArray();
    for (auto& request : base_requests_arr) {
        const auto& request_map = request.AsMap();
        const auto& type = request_map.at("type").AsString();
        if (type == "Stop") {
            auto [stop_name, coordinates] = PopulateStop(request_map);
            catalogue.AddStop(string{stop_name}, coordinates);
        }
    }
    PopulateStopDistances(catalogue);  
    for (auto& request : base_requests_arr) {
        const auto& request_map = request.AsMap();
        const auto& type = request_map.at("type").AsString();
        if (type == "Bus") {
            auto [bus_name, stops, is_roundtrip] = PopulateBus(request_map);
            catalogue.AddBus(string{bus_name}, stops, is_roundtrip);
        }
    }
}

pair<string_view, geo::Coordinates> JsonReader::PopulateStop(const json::Dict& request_map) const {
    string_view stop_name = request_map.at("name").AsString();
    geo::Coordinates coordinates = {request_map.at("latitude").AsDouble(), request_map.at("longitude").AsDouble()};
    return {stop_name, coordinates};
}

void JsonReader::PopulateStopDistances(transport_catalogue::TransportCatalogue& catalogue) const {
    const json::Array& base_requests_arr = GetBaseRequests().AsArray();
    for (auto& request : base_requests_arr) {
        const auto& request_map = request.AsMap();
        const auto& type = request_map.at("type").AsString();
        if (type == "Stop") {
            string_view stop1 = request_map.at("name").AsString();
            auto& distances = request_map.at("road_distances").AsMap();
            for (auto& [stop2, dist] : distances) {
                catalogue.SetDistance(stop1, stop2, dist.AsInt());
            }
        }
    }
}

tuple<string_view, vector<string_view>, bool> JsonReader::PopulateBus(const json::Dict& request_map) const {
    string_view bus_name = request_map.at("name").AsString();
    vector<string_view> stops;
    bool is_roundtrip = request_map.at("is_roundtrip").AsBool();
    const auto& stops_arr = request_map.at("stops").AsArray();
    for (auto& stop : stops_arr) {
        stops.push_back(stop.AsString());
    }
    if (!is_roundtrip) {
        for (int i = stops_arr.size() - 2; i >= 0; --i) {
            stops.push_back(stops_arr[i].AsString());
        }
    }
    return make_tuple(bus_name, stops, is_roundtrip);
}

variant<monostate, string, svg::Rgb, svg::Rgba> JsonReader::ParseColor(const json::Node& color_node) const {
    if (color_node.IsString()) {
        return color_node.AsString();
    } else if (color_node.IsArray()) {
        const auto& color_array = color_node.AsArray();
        if (color_array.size() == 3) {
            return svg::Rgb{static_cast<uint8_t>(color_array[0].AsInt())
                            , static_cast<uint8_t>(color_array[1].AsInt())
                            , static_cast<uint8_t>(color_array[2].AsInt())};
        } else if (color_array.size() == 4) {
            return svg::Rgba{static_cast<uint8_t>(color_array[0].AsInt())
                            , static_cast<uint8_t>(color_array[1].AsInt())
                            , static_cast<uint8_t>(color_array[2].AsInt())
                            , color_array[3].AsDouble()};
        } else {
            throw std::logic_error("Invalid color format: expected 3 or 4 elements in array");
        }
    } else {
        throw std::logic_error("Invalid color type: expected string or array");
    }
}

map_renderer::MapRenderer JsonReader::FillRenderSettings(const json::Dict& request_map) const {
    map_renderer::RenderSettings render_settings;
    render_settings.width = request_map.at("width").AsDouble();
    render_settings.height = request_map.at("height").AsDouble();
    render_settings.padding = request_map.at("padding").AsDouble();
    render_settings.stop_radius = request_map.at("stop_radius").AsDouble();
    render_settings.line_width = request_map.at("line_width").AsDouble();
    render_settings.bus_label_font_size = request_map.at("bus_label_font_size").AsInt();

    const json::Array& bus_label_offset = request_map.at("bus_label_offset").AsArray();
    render_settings.bus_label_offset = {bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble()};

    render_settings.stop_label_font_size = request_map.at("stop_label_font_size").AsInt();

    const json::Array& stop_label_offset = request_map.at("stop_label_offset").AsArray();
    render_settings.stop_label_offset = {stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble()};

    render_settings.underlayer_color = ParseColor(request_map.at("underlayer_color"));
    render_settings.underlayer_width = request_map.at("underlayer_width").AsDouble();
    
    const json::Array& color_palette = request_map.at("color_palette").AsArray();
    for (const auto& color_element : color_palette) {
        render_settings.color_palette.push_back(ParseColor(color_element));
    }
    
    return render_settings;
}
