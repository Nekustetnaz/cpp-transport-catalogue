#include "request_handler.h"

#include <algorithm>
#include <map>
#include <sstream>

using namespace std;

bool RequestHandler::IsBusExists(std::string_view bus_name) const {
    return catalogue_.FindBus(bus_name);
}

bool RequestHandler::IsStopExists(std::string_view stop_name) const {
    return catalogue_.FindStop(stop_name);
}

const domain::RouteInfo RequestHandler::GetRouteInfo(std::string_view bus_name) const {
    const auto& bus = catalogue_.FindBus(bus_name);
    return catalogue_.GetRouteInfo(bus);
}

const std::vector<std::string_view> RequestHandler::GetBuses(std::string_view stop_name) const {
    const unordered_set<string_view>& buses = catalogue_.GetBusesToStop(stop_name);
    vector<string_view> buses_vector(buses.begin(), buses.end());
    sort(buses_vector.begin(), buses_vector.end());
    return buses_vector;  
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

    return renderer_.CreateSVG(stops, sorted_buses);
}

const std::optional<vector<const graph::Edge<double>*>> RequestHandler::GetBestRoute(
    string_view stop_from, std::string_view stop_to) const {
    return router_.GetRoute(stop_from, stop_to);
}
