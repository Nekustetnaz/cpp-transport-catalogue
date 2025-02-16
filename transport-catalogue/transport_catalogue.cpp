#include <string>
#include <string_view>
#include <set>
#include <unordered_set>
#include <vector>

#include "transport_catalogue.h"

using namespace std;

namespace transport_catalogue {

void TransportCatalogue::AddStop(string name, geo::Coordinates coordinates) {
    all_stops_.push_back({move(name), move(coordinates)});
    name_to_stop_[all_stops_.back().name] = &all_stops_.back();
}

void TransportCatalogue::AddBus(string name, vector<string> stops) {
    all_buses_.push_back({move(name), move(stops)});
    name_to_bus_[all_buses_.back().name] = &all_buses_.back();

    for (const string& stop : all_buses_.back().stops) {
        stop_to_buses_[stop].insert(all_buses_.back().name);
    }
}

const TransportCatalogue::Bus* TransportCatalogue::FindBus(const string_view name) const {
    return name_to_bus_.count(name) > 0 ? name_to_bus_.at(name) : nullptr;
}

const TransportCatalogue::Stop* TransportCatalogue::FindStop(const string_view name) const {
    return name_to_stop_.count(name) > 0 ? name_to_stop_.at(name) : nullptr;
}

const set<string_view> TransportCatalogue::GetBusesToStop(const string_view stop_name) const {
    return stop_to_buses_.count(stop_name) > 0 ? stop_to_buses_.at(stop_name) : set<string_view>();
}

const TransportCatalogue::RouteInfo TransportCatalogue::GetRouteInfo(const TransportCatalogue::Bus* bus) const {
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