#include <iostream>

#include "json_reader.h"

using namespace std;

int main() {
    /*
     * Примерная структура программы:
     *
     * Считать JSON из stdin
     * Построить на его основе JSON базу данных транспортного справочника
     * Выполнить запросы к справочнику, находящиеся в массиве "stat_requests", построив JSON-массив
     * с ответами.
     * Вывести в stdout ответы в виде JSON
     */

    using namespace transport;
    
    TransportCatalogue catalogue;

    json_reader::ProcessJSON(catalogue, cin, cout);
}

