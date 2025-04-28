#pragma once

#include "domain.h"
#include "geo.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace transport_catalogue {

class TransportCatalogue {
public:
    struct StopDistancesHasher {
        size_t operator()(const std::pair<const domain::Stop*, const domain::Stop*>& stops) const {
            size_t hash_first = std::hash<const void*>{}(stops.first);
            size_t hash_second = std::hash<const void*>{}(stops.second);
            return hash_first + hash_second * 37;
        }
    };

    void AddStop(const std::string& name, const geo::Coordinates coordinates);
    void SetDistance(const std::string_view stop_from, const std::string_view stop_to, int length);
    void AddBus(const std::string& name, const std::vector<std::string_view>& stops, bool is_roundtrip);

    const domain::Bus* FindBus(const std::string_view name) const;
    const domain::Stop* FindStop(const std::string_view name) const;
    int GetDistance(const domain::Stop* stop_from, const domain::Stop* stop_to) const;

    const std::unordered_set<std::string_view>& GetBusesToStop(const std::string_view stop_name) const;
    const domain::RouteInfo GetRouteInfo(const domain::Bus* bus) const;

    const std::unordered_map<std::string_view, const domain::Stop*>& GetAllStops() const;
    const std::unordered_map<std::string_view, const domain::Bus*>& GetAllBuses() const;  

private:
    std::deque<domain::Stop> all_stops_;
    std::deque<domain::Bus> all_buses_;

    std::unordered_map<std::string_view, const domain::Stop*> name_to_stop_;
    std::unordered_map<std::string_view, const domain::Bus*> name_to_bus_;
    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_to_buses_;

    std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, StopDistancesHasher> stop_route_length_;
};

}