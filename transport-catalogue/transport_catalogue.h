#pragma once

#include <deque>
#include <string>
#include <unordered_map>
#include <set>
#include <vector>

#include "geo.h"

namespace transport_catalogue {

class TransportCatalogue {
public:
	struct Bus {
    	std::string name;
    	std::vector<std::string> stops;
	};

	struct Stop {
    	std::string name;
    	geo::Coordinates coordinates;
	};

	struct RouteInfo {
    	size_t stops_number = 0;
    	size_t unique_stops_number = 0;
    	double distance = 0;
	};

	void AddStop(const std::string name, const geo::Coordinates coordinates);
	void AddBus(const std::string name, const std::vector<std::string> stops);
	const Bus* FindBus(const std::string_view name) const;
	const Stop* FindStop(const std::string_view name) const;
	const std::set<std::string_view> GetBusesToStop(const std::string_view stop_name) const;
	const RouteInfo GetRouteInfo(const Bus* bus) const;

private:
	std::deque<Stop> all_stops_;
    std::deque<Bus> all_buses_;

	std::unordered_map<std::string_view, const Stop*> name_to_stop_;
    std::unordered_map<std::string_view, const Bus*> name_to_bus_;
	std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_buses_;
};

}
