#pragma once

#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

namespace transport_catalogue {

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};

struct Bus {
    std::string name;
    std::vector<std::string_view> stops;
};

struct RouteInfo {
    size_t stops_number = 0;
    size_t unique_stops_number = 0;
    double distance = 0;
    int route_length = 0;
    double curvature = 0;
};

class TransportCatalogue {
public:
    struct StopDistancesHasher {
        size_t operator()(const std::pair<const Stop*, const Stop*>& stops) const {
            size_t hash_first = std::hash<const void*>{}(stops.first);
            size_t hash_second = std::hash<const void*>{}(stops.second);
            return hash_first + hash_second * 37;
        }
    };

    void AddStop(const std::string& name, const geo::Coordinates coordinates);
    void SetDistance(const std::string_view stop1, const std::string_view stop2, int length);
    void AddBus(const std::string& name, const std::vector<std::string_view>& stops);
    const Bus* FindBus(const std::string_view name) const;
    const Stop* FindStop(const std::string_view name) const;
    const std::unordered_set<std::string_view>& GetBusesToStop(const std::string_view stop_name) const;
    const RouteInfo GetRouteInfo(const Bus* bus) const;

private:
    std::deque<Stop> all_stops_;
    std::deque<Bus> all_buses_;

    std::unordered_map<std::string_view, const Stop*> name_to_stop_;
    std::unordered_map<std::string_view, const Bus*> name_to_bus_;
    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_to_buses_;

    std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopDistancesHasher> stop_route_length_;
};

}