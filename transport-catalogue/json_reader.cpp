#include "json_reader.h"

#include <algorithm>
#include <sstream>

using namespace std;

const json::Node& JsonReader::GetBaseRequests() const {
    auto br_iter = input_.GetRoot().AsDict().find("base_requests");
    return br_iter != input_.GetRoot().AsDict().end() ? br_iter -> second : dummy_;
}

const json::Node& JsonReader::GetStatRequests() const {
    auto sr_iter = input_.GetRoot().AsDict().find("stat_requests");
    return sr_iter != input_.GetRoot().AsDict().end() ? sr_iter -> second : dummy_;
}

const json::Node& JsonReader::GetRenderSettings() const {
    auto rs_iter = input_.GetRoot().AsDict().find("render_settings");
    return rs_iter != input_.GetRoot().AsDict().end() ? rs_iter -> second : dummy_;
}

const json::Node& JsonReader::GetRoutingSettings() const {
    auto rs_iter = input_.GetRoot().AsDict().find("routing_settings");
    return rs_iter != input_.GetRoot().AsDict().end() ? rs_iter -> second : dummy_;
}

void JsonReader::PopulateCatalogue(transport_catalogue::TransportCatalogue& catalogue) {
    const json::Array& base_requests_arr = GetBaseRequests().AsArray();
    PopulateStop(base_requests_arr, catalogue);  
    PopulateStopDistances(base_requests_arr, catalogue); 
    PopulateBus(base_requests_arr, catalogue); 
}

pair<string_view, geo::Coordinates> JsonReader::ParseStop(const json::Dict& request_map) const {
    string_view stop_name = request_map.at("name").AsString();
    geo::Coordinates coordinates = {request_map.at("latitude").AsDouble(), request_map.at("longitude").AsDouble()};
    return {stop_name, coordinates};
}

void JsonReader::PopulateStop(const json::Array& base_requests_arr, transport_catalogue::TransportCatalogue& catalogue) const {
    for (auto& request : base_requests_arr) {
        const auto& request_map = request.AsDict();
        const auto& type = request_map.at("type").AsString();
        if (type == "Stop") {
            auto [stop_name, coordinates] = ParseStop(request_map);
            catalogue.AddStop(string{stop_name}, coordinates);
        }
    }
}

void JsonReader::PopulateStopDistances(const json::Array& base_requests_arr, transport_catalogue::TransportCatalogue& catalogue) const {
    for (auto& request : base_requests_arr) {
        const auto& request_map = request.AsDict();
        const auto& type = request_map.at("type").AsString();
        if (type == "Stop") {
            string_view stop_from = request_map.at("name").AsString();
            auto& distances = request_map.at("road_distances").AsDict();
            for (auto& [stop_to, dist] : distances) {
                catalogue.SetDistance(stop_from, stop_to, dist.AsInt());
            }
        }
    }
}

tuple<string_view, vector<string_view>, bool> JsonReader::ParseBus(const json::Dict& request_map) const {
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

void JsonReader::PopulateBus(const json::Array& base_requests_arr, transport_catalogue::TransportCatalogue& catalogue) const {
    for (auto& request : base_requests_arr) {
        const auto& request_map = request.AsDict();
        const auto& type = request_map.at("type").AsString();
        if (type == "Bus") {
            auto [bus_name, stops, is_roundtrip] = ParseBus(request_map);
            catalogue.AddBus(string{bus_name}, stops, is_roundtrip);
        }
    }
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

transport_router::TransportRouter JsonReader::FillRoutingSettings(
    const json::Dict& request_map
    , const transport_catalogue::TransportCatalogue& catalogue
    ) const {
    transport_router::RoutingSettings routing_settings;
    routing_settings.bus_wait_time = request_map.at("bus_wait_time"s).AsInt();
    routing_settings.bus_velocity = request_map.at("bus_velocity"s).AsDouble();
    return {routing_settings, catalogue};
}

void JsonReader::ProcessStatRequests(const json::Node& stat_requests, const RequestHandler& handler) const {
    json::Array result;
    for (auto& request : stat_requests.AsArray()) {
        const auto& request_map = request.AsDict();
        const auto& type = request_map.at("type").AsString();
        if (type == "Bus") {
            result.push_back(PrintBus(request_map, handler).AsDict());
        }        
        if (type == "Stop") {
            result.push_back(PrintStop(request_map, handler).AsDict());
        }
        if (type == "Map") {
            result.push_back(PrintMap(request_map, handler).AsDict());
        }
        if (type == "Route") {
            result.push_back(PrintBestRoute(request_map, handler).AsDict());
        }
    }
    json::Print(json::Document{result}, cout);
}

const json::Node JsonReader::PrintBus(const json::Dict& request_map, const RequestHandler& handler) const {
    json::Node result;
    const int request_id = request_map.at("id").AsInt();
    const string& bus_name = request_map.at("name").AsString();
    if (!handler.IsBusExists(bus_name)) {
        result = PrintNotFoundError(request_id);
    }
    else {
        const domain::RouteInfo route = handler.GetRouteInfo(bus_name);
        result = json::Builder{}
                        .StartDict()
                            .Key("request_id").Value(request_id)
                            .Key("curvature").Value(route.curvature)
                            .Key("route_length").Value(route.route_length)
                            .Key("stop_count").Value(static_cast<int>(route.stops_number))
                            .Key("unique_stop_count").Value(static_cast<int>(route.unique_stops_number))
                        .EndDict()
                    .Build();
    }
    return result;
}

const json::Node JsonReader::PrintStop(const json::Dict& request_map, const RequestHandler& handler) const {
    json::Node result;
    const int request_id = request_map.at("id").AsInt();
    const string& stop_name = request_map.at("name").AsString();
    if (!handler.IsStopExists(stop_name)) {
        result = PrintNotFoundError(request_id);
    }
    else {
        json::Array buses;
        for (const string_view bus : handler.GetBuses(stop_name)) {
            buses.push_back(string{bus});
        }
        result = json::Builder{}
                        .StartDict()
                            .Key("request_id").Value(request_id)
                            .Key("buses").Value(buses)
                        .EndDict()
                    .Build();
    }
    return result;
}

const json::Node JsonReader::PrintMap(const json::Dict& request_map, const RequestHandler& handler) const {
    json::Node result;
    const int request_id = request_map.at("id").AsInt();
    ostringstream strm;
    svg::Document svg_map = handler.RenderMap();
    svg_map.Render(strm);
    result = json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(request_id)
                        .Key("map").Value(strm.str())
                    .EndDict()
                .Build();
    return result;
}

const json::Node JsonReader::PrintNotFoundError(const int request_id) const {
    return json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(request_id)
                        .Key("error_message").Value("not found")
                    .EndDict()
                .Build();
}

const json::Node JsonReader::PrintBestRoute(const json::Dict& request_map, const RequestHandler& handler) const {
    json::Node result;
    const int request_id = request_map.at("id"s).AsInt();
    const string_view stop_from = request_map.at("from"s).AsString();
    const string_view stop_to = request_map.at("to"s).AsString();
    const auto& route = handler.GetBestRoute(stop_from, stop_to);
    
    if (!route) {
        result = PrintNotFoundError(request_id);
    }
    else {
        json::Array items;
        double total_time = 0.0;
        items.reserve(route.value().edges.size());
        for (auto& edge_id : route.value().edges) {
            const graph::Edge<double> edge = handler.GetRouteGraph().GetEdge(edge_id);
            if (edge.quality == 0) {
                items.emplace_back(json::Node(json::Builder{}
                    .StartDict()
                        .Key("stop_name"s).Value(edge.name)
                        .Key("time"s).Value(edge.weight)
                        .Key("type"s).Value("Wait"s)
                    .EndDict()
                .Build()));

                total_time += edge.weight;
            }
            else {
                items.emplace_back(json::Node(json::Builder{}
                    .StartDict()
                        .Key("bus"s).Value(edge.name)
                        .Key("span_count"s).Value(static_cast<int>(edge.quality))
                        .Key("time"s).Value(edge.weight)
                        .Key("type"s).Value("Bus"s)
                    .EndDict()
                .Build()));

                total_time += edge.weight;
            }
        }

        result = json::Builder{}
            .StartDict()
                .Key("request_id"s).Value(request_id)
                .Key("total_time"s).Value(total_time)
                .Key("items"s).Value(items)
            .EndDict()
        .Build();
    }

    return result;
}