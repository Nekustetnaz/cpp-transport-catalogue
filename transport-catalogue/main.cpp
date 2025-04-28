#include "json_reader.h"
#include "request_handler.h"

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    JsonReader requests(std::cin);
    requests.PopulateCatalogue(catalogue);

    const auto& stat_requests = requests.GetStatRequests();
    const auto& render_settings = requests.GetRenderSettings().AsDict();
    const auto& renderer = requests.FillRenderSettings(render_settings);
    const auto& routing_settings = requests.GetRoutingSettings().AsDict();
    const auto& transport_router = requests.FillRoutingSettings(routing_settings, catalogue);

    RequestHandler handler(renderer, catalogue, transport_router);

    requests.ProcessStatRequests(stat_requests, handler);
}