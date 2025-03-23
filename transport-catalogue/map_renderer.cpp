#include "map_renderer.h"

namespace map_renderer {

using namespace std;

inline const double EPSILON = 1e-6;

bool IsZero(double value) {
    return abs(value) < EPSILON;
}

vector<svg::Polyline> MapRenderer::GetRouteLines(const map<string_view, const domain::Bus*>& buses, const SphereProjector& sp) const {
    vector<svg::Polyline> result;
    size_t color = 0;
    for (const auto& [bus_name, bus] : buses) {
        if (bus->stops.empty()) {
            continue;
        }

        svg::Polyline line;
        vector<const domain::Stop*> route_stops{bus->stops.begin(), bus->stops.end()};
        for (const auto& stop : route_stops) {
            line.AddPoint(sp(stop->coordinates));
        }

        line.SetStrokeColor(render_settings_.color_palette[color]);
        line.SetFillColor("none");
        line.SetStrokeWidth(render_settings_.line_width);
        line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        if (color < (render_settings_.color_palette.size() - 1)) {
            ++color;
        } else {
            color = 0;
        }
        result.push_back(line);
    }    
    return result;
}

vector<svg::Text> MapRenderer::GetBusName(const map<string_view, const domain::Bus*>& buses, const SphereProjector& sp) const {
    vector<svg::Text> result;
    size_t color_num = 0;
    svg::Text text;
    svg::Text underlayer;

    for (const auto& [bus_number, bus] : buses) {
        if (bus->stops.empty()) {
            continue;
        }

        text.SetPosition(sp(bus->stops[0]->coordinates));
        text.SetOffset(render_settings_.bus_label_offset);
        text.SetFontSize(render_settings_.bus_label_font_size);
        text.SetFontFamily("Verdana");
        text.SetFontWeight("bold");
        text.SetData(bus->name);
        text.SetFillColor(render_settings_.color_palette[color_num]);

        if (color_num < (render_settings_.color_palette.size() - 1)) {
            ++color_num;
        } else {
            color_num = 0;
        }

        underlayer.SetPosition(sp(bus->stops[0]->coordinates));
        underlayer.SetOffset(render_settings_.bus_label_offset);
        underlayer.SetFontSize(render_settings_.bus_label_font_size);
        underlayer.SetFontFamily("Verdana");
        underlayer.SetFontWeight("bold");
        underlayer.SetData(bus->name);
        underlayer.SetFillColor(render_settings_.underlayer_color);
        underlayer.SetStrokeColor(render_settings_.underlayer_color);
        underlayer.SetStrokeWidth(render_settings_.underlayer_width);
        underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        result.push_back(underlayer);
        result.push_back(text);

        if (bus->is_roundtrip == false && bus->stops.size() > 1 && bus->stops[0] != bus->stops[(bus->stops.size() - 1) / 2]) {
            svg::Text text2{text};
            svg::Text underlayer2{underlayer};
            text2.SetPosition(sp(bus->stops[(bus->stops.size() - 1) / 2]->coordinates));
            underlayer2.SetPosition(sp(bus->stops[(bus->stops.size() - 1) / 2]->coordinates));
            
            result.push_back(underlayer2);
            result.push_back(text2);
        }
    }
    
    return result;
}

vector<svg::Circle> MapRenderer::GetStopMarks(map<string_view, const domain::Stop*>& stops, const SphereProjector& sp) const {
    vector<svg::Circle> result;
    for (const auto& [stop_name, stop] : stops) {
        svg::Circle symbol;
        symbol.SetCenter(sp(stop->coordinates));
        symbol.SetRadius(render_settings_.stop_radius);
        symbol.SetFillColor("white");        
        result.push_back(symbol);
    }
    
    return result;
}

vector<svg::Text> MapRenderer::GetStopNames(map<string_view, const domain::Stop*>& stops, const SphereProjector& sp) const {
    vector<svg::Text> result;
    svg::Text text;
    svg::Text underlayer;

    for (const auto& [stop_name, stop] : stops) {
        text.SetPosition(sp(stop->coordinates));
        text.SetOffset(render_settings_.stop_label_offset);
        text.SetFontSize(render_settings_.stop_label_font_size);
        text.SetFontFamily("Verdana");
        text.SetData(stop->name);
        text.SetFillColor("black");
        
        underlayer.SetPosition(sp(stop->coordinates));
        underlayer.SetOffset(render_settings_.stop_label_offset);
        underlayer.SetFontSize(render_settings_.stop_label_font_size);
        underlayer.SetFontFamily("Verdana");
        underlayer.SetData(stop->name);
        underlayer.SetFillColor(render_settings_.underlayer_color);
        underlayer.SetStrokeColor(render_settings_.underlayer_color);
        underlayer.SetStrokeWidth(render_settings_.underlayer_width);
        underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        result.push_back(underlayer);
        result.push_back(text);
    }
    
    return result;
}

svg::Document MapRenderer::GetSVG(const vector<const domain::Stop*>& stops, const map<string_view, const domain::Bus*>& buses) const {
    svg::Document result;
    vector<geo::Coordinates> stops_coord;
    map<string_view, const domain::Stop*> sorted_stops;

    for (const auto& stop : stops) {
        stops_coord.push_back(stop->coordinates);
        sorted_stops.insert({stop->name, stop});
    }
    SphereProjector sp_proj(stops_coord.begin(), stops_coord.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
    for (const auto& line : GetRouteLines(buses, sp_proj)) {
        result.Add(line);
    }
    for (const auto& bus_name : GetBusName(buses, sp_proj)) {
        result.Add(bus_name);
    }
    for (const auto& stop_mark : GetStopMarks(sorted_stops, sp_proj)) {
        result.Add(stop_mark);
    }
    for (const auto& stop_name : GetStopNames(sorted_stops, sp_proj)) {
        result.Add(stop_name);
    }
    return result;
}

}