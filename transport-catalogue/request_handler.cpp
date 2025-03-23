#include "request_handler.h"

#include <algorithm>
#include <map>
#include <sstream>

using namespace std;

void RequestHandler::ProcessRequests(const json::Node& stat_requests) const {
    json::Array result;
    for (auto& request : stat_requests.AsArray()) {
        const auto& request_map = request.AsMap();
        const auto& type = request_map.at("type").AsString();
        if (type == "Bus") {
            result.push_back(PrintBus(request_map).AsMap());
        }        
        if (type == "Stop") {
            result.push_back(PrintStop(request_map).AsMap());
        }
        if (type == "Map") {
            result.push_back(PrintMap(request_map).AsMap());
        }
    }
    json::Print(json::Document{result}, cout);
}

const json::Node RequestHandler::PrintBus(const json::Dict& request_map) const {
    json::Dict result;
    result["request_id"] = request_map.at("id").AsInt();
    const string& route_number = request_map.at("name").AsString();
    const auto& bus = catalogue_.FindBus(route_number);
    if (catalogue_.FindBus(route_number) == nullptr) {
        result["error_message"] = json::Node{static_cast<string>("not found")};
    }
    else {
        domain::RouteInfo route = catalogue_.GetRouteInfo(bus);
        result["curvature"] = route.curvature;
        result["route_length"] = route.route_length;
        result["stop_count"] = static_cast<int>(route.stops_number);
        result["unique_stop_count"] = static_cast<int>(route.unique_stops_number);
    }
    return json::Node{result};
}

const json::Node RequestHandler::PrintStop(const json::Dict& request_map) const {
    json::Dict result;
    result["request_id"] = request_map.at("id").AsInt();
    const string& stop_name = request_map.at("name").AsString();
    if (catalogue_.FindStop(stop_name) == nullptr) {
        result["error_message"] = json::Node{static_cast<string>("not found")};
    }
    else {
        const unordered_set<string_view>& buses = catalogue_.GetBusesToStop(stop_name);
        vector<string_view> buses_vector(buses.begin(), buses.end());
        sort(buses_vector.begin(), buses_vector.end());
        json::Array buses_arr;
        for (const string_view bus : buses_vector) {
            buses_arr.push_back(string{bus});
        }
        result["buses"] = buses_arr;
    }

    return json::Node{result};
}

const json::Node RequestHandler::PrintMap(const json::Dict& request_map) const {
    json::Dict result;
    result["request_id"] = request_map.at("id").AsInt();
    ostringstream strm;
    svg::Document map = RenderMap();
    map.Render(strm);
    result["map"] = strm.str();

    return json::Node{result};
}

svg::Document RequestHandler::RenderMap() const {
    const unordered_map<string_view, const domain::Bus*>& all_buses = catalogue_.GetAllBuses();
    vector<const domain::Stop*> stops;
    for (const auto& [bus_number, bus] : all_buses) {
        for (const domain::Stop* stop : bus->stops)
        stops.push_back(stop);
    }
    map<string_view, const domain::Bus*> sorted_buses;
    for (const auto& bus : all_buses) {
        sorted_buses.emplace(bus);
    }

    return renderer_.GetSVG(stops, sorted_buses);
}