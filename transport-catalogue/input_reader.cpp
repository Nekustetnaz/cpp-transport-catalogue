#include <algorithm>
#include <cassert>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

#include "input_reader.h"

geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);
    auto comma2 = str.find(',', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = comma2 == str.npos ? std::stod(std::string(str.substr(not_space2))) 
                                    : std::stod(std::string(str.substr(not_space2, comma2 - not_space2)));

    return {lat, lng};
}

std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

std::unordered_map<std::string_view, int> ParseDistance(std::string_view str) {
    std::unordered_map<std::string_view, int> length_to_stops;
    
    auto comma1 = str.find(',');
    auto comma2 = str.find(',', comma1 + 1);
    if (comma2 == str.npos) {
        return length_to_stops;
    }

    auto lengths = Split(str.substr(comma2 + 1), ',');
    for (std::string_view length : lengths) {
        auto letter_m = length.find('m');
        length_to_stops.insert({length.substr(letter_m + 5), std::stoi(std::string(length.substr(0, letter_m)))});
    }

    return length_to_stops;
}

void InputReader::ApplyCommands([[maybe_unused]] transport_catalogue::TransportCatalogue& catalogue) const {
    if (commands_.empty()) {
        return;
    }
    for (auto& command : commands_) {
        if (command.command == "Stop") {
            geo::Coordinates coords = {ParseCoordinates(command.description)};
            catalogue.AddStop(command.id, coords);
        }
    }
    for (auto& command : commands_) {
        if (command.command == "Stop") {
            const std::unordered_map<std::string_view, int>& length_to_stops = ParseDistance(command.description);
            if (length_to_stops.empty()) {
                continue;
            }
            for (const auto [stop, length] : length_to_stops) {
                catalogue.SetDistance(command.id, stop, length);
            }
        }
    }
    for (auto& command : commands_) {
        if (command.command == "Bus") {
            catalogue.AddBus(command.id, ParseRoute(command.description));
        }
    }
}