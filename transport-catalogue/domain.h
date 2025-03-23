#pragma once

#include "geo.h"

#include <string>
#include <vector>

namespace domain {

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> stops;
    bool is_roundtrip;
};

struct RouteInfo {
    size_t stops_number = 0;
    size_t unique_stops_number = 0;
    double distance = 0;
    int route_length = 0;
    double curvature = 0;
};

}