#include <iostream>
#include <string>
#include <set>

#include "stat_reader.h"

using namespace std::literals;
using namespace transport_catalogue;

void PrintBus (const TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& output, 
               std::string_view command_id) {
    const auto& bus = tansport_catalogue.FindBus(command_id);
    if (bus == nullptr) {
        output << request << ": not found"s << std::endl;
        return;
    }
    const TransportCatalogue::RouteInfo route = tansport_catalogue.GetRouteInfo(bus);
    output << request << ": "s << route.stops_number << " stops on route, "s 
        << route.unique_stops_number << " unique stops, "s 
        << route.distance << " route length"s << std::endl;
}

void PrintStop (const TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& output, 
                std::string_view command_id) {
    if (tansport_catalogue.FindStop(command_id) == nullptr) {
        output << request << ": not found"s << std::endl;
        return;
    }
    const std::set<std::string_view>& buses = tansport_catalogue.GetBusesToStop(command_id);
    if (buses.empty()) {
        output << request << ": no buses"s << std::endl;
        return;
    }
    output << request << ": buses"s;
    for (const std::string_view bus : buses) {
        output << " "s << bus;
    }
    output << std::endl;
}

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output) {
    auto space_pos = request.find(' ');
    if (space_pos == request.npos) {
        return;
    }
    const std::string_view command = request.substr(0, space_pos);
    const std::string_view command_id = request.substr(space_pos + 1);

    if (command == "Bus") {
        PrintBus(tansport_catalogue, request, output, command_id);
    }
    if (command == "Stop") {
        PrintStop(tansport_catalogue, request, output, command_id);
    }
}
