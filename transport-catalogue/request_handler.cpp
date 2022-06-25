#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

using namespace std;

namespace transport {

 RequestHandler::RequestHandler(const TransportCatalogue& db, renderer::MapRenderer& renderer) 
    : db_(db), renderer_(renderer) {

}

optional<BusStat> RequestHandler::GetBusStat(const string& bus_name) const {
    return db_.GetBusStat(bus_name);
}

const set<string_view>* RequestHandler::GetBusesThroughStop(const string& stop_name) const {
    return db_.GetBusesThroughStop(stop_name);
}

const svg::Document& RequestHandler::RenderMap() const {
    renderer_.AddLinesToSvg();
    renderer_.AddBusLabelsToSvg();
    renderer_.AddStopSymToSvg();
    renderer_.AddStopLabelsToSvg();

    return renderer_.GetSvgDoc();
}

} // transport