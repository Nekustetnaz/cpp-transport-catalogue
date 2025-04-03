#include "json_reader.h"
#include "request_handler.h"

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    JsonReader requests(std::cin);
    requests.PopulateCatalogue(catalogue);

    const auto& stat_requests = requests.GetStatRequests();
    const auto& render_settings = requests.GetRenderSettings().AsDict();
    const auto& renderer = requests.FillRenderSettings(render_settings);

    RequestHandler handler(renderer, catalogue);

    requests.ProcessStatRequests(stat_requests, handler);
}
