#include "transport_catalogue.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace transport_catalogue {

using namespace std;
using namespace domain;

void TransportCatalogue::AddStop(const string& name, const geo::Coordinates coordinates) {
    all_stops_.push_back({move(name), move(coordinates)});
    name_to_stop_[all_stops_.back().name] = &all_stops_.back();
}


void TransportCatalogue::SetDistance(const string_view stop1, const string_view stop2, int length) {
    stop_route_length_.insert({{name_to_stop_.at(stop1), name_to_stop_.at(stop2)}, length});
}

void TransportCatalogue::AddBus(const string& name, const vector<string_view>& stops, bool is_roundtrip) {
    vector<const Stop*> bus_stops;
    for (const string_view stop : stops) {
        bus_stops.push_back(name_to_stop_.at(stop));
    }
    all_buses_.push_back({move(name), move(bus_stops), is_roundtrip});
    name_to_bus_[all_buses_.back().name] = &all_buses_.back();

    for (const Stop* stop : all_buses_.back().stops) {
        stop_to_buses_[stop->name].insert(all_buses_.back().name);
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

const unordered_set<string_view>& TransportCatalogue::GetBusesToStop(const string_view stop_name) const {
    auto stop_iter = stop_to_buses_.find(stop_name);
    static const unordered_set<string_view> empty_result;
    return stop_iter != stop_to_buses_.end() ? stop_iter->second : empty_result;
}

const RouteInfo TransportCatalogue::GetRouteInfo(const Bus* bus) const {
    RouteInfo route;
    unordered_set<string_view> unique_stops;
    geo::Coordinates current_stop_coordinates;
    string_view current_stop;
    bool is_first_stop = true;
    for (const Stop* stop : bus->stops) {
        ++route.stops_number;
        unique_stops.insert(stop->name);
        if (is_first_stop) {
            current_stop_coordinates = stop->coordinates;
            current_stop = stop->name;
            is_first_stop = false;
            continue;
        }
        route.distance += geo::ComputeDistance(stop->coordinates, current_stop_coordinates);
        auto route_length_iter = stop_route_length_.find({name_to_stop_.at(current_stop), stop});
        if (route_length_iter != stop_route_length_.end()) {
            route.route_length += route_length_iter->second;
        } else {
            route.route_length += stop_route_length_.at({stop, name_to_stop_.at(current_stop)});
        }
        current_stop_coordinates = stop->coordinates;
        current_stop = stop->name;
    }
    route.unique_stops_number = unique_stops.size();
    route.curvature = route.route_length / route.distance; 
    return route;
}

const unordered_map<string_view, const Stop*>& TransportCatalogue::GetAllStops() const {
    return name_to_stop_;
}

const unordered_map<string_view, const Bus*>& TransportCatalogue::GetAllBuses() const {
    return name_to_bus_;
}

}