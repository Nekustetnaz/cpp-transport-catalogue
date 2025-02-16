#include <algorithm>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "transport_catalogue.h"

using namespace std;

namespace transport_catalogue {

void TransportCatalogue::AddStop(const string& name, const geo::Coordinates coordinates) {
    all_stops_.push_back({move(name), move(coordinates)});
    name_to_stop_[all_stops_.back().name] = &all_stops_.back();
}

void TransportCatalogue::AddBus(const string& name, const vector<string_view> stops) {
    vector<string> stops_to_string;
    for (string_view stop : stops) {
        stops_to_string.push_back(string(stop));
    }
    all_buses_.push_back({move(name), move(stops_to_string)});
    name_to_bus_[all_buses_.back().name] = &all_buses_.back();

    for (const string& stop : all_buses_.back().stops) {
        stop_to_buses_[stop].insert(all_buses_.back().name);
    }
}

const Bus* TransportCatalogue::FindBus(const string_view name) const {
    auto bus_iter = name_to_bus_.find(name);
    return bus_iter != name_to_bus_.end() ? bus_iter->second : nullptr;
}

const Stop* TransportCatalogue::FindStop(const string_view name) const {
    auto stop_iter = name_to_stop_.find(name);
    return stop_iter != name_to_stop_.end() ? stop_iter->second : nullptr;
}

const unordered_set<string_view>* TransportCatalogue::GetBusesToStop(const string_view stop_name) const {
    auto stop_iter = stop_to_buses_.find(stop_name);
    return stop_iter != stop_to_buses_.end() ? &stop_iter->second : nullptr;
}

const RouteInfo TransportCatalogue::GetRouteInfo(const Bus* bus) const {
    RouteInfo route;
    unordered_set<string_view> unique_stops;
    geo::Coordinates current_stop_coordinates;
    bool is_first_stop = true;
    for (string_view stop : bus->stops) {
        ++route.stops_number;
        unique_stops.insert(stop);
        if (is_first_stop) {
            current_stop_coordinates = name_to_stop_.at(stop)->coordinates;
            is_first_stop = false;
            continue;
        }
        route.distance += geo::ComputeDistance(name_to_stop_.at(stop)->coordinates, current_stop_coordinates);
        current_stop_coordinates = name_to_stop_.at(stop)->coordinates;
    }
    route.unique_stops_number = unique_stops.size();
    return route;
}

}