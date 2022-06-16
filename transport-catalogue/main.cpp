#include <iostream>
#include <sstream>

#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"

//#define ISSTR ;

using namespace std::literals;

int main()
{

#ifdef ISSTR

    std::istringstream jinput{
"{"
"  \"base_requests\": [ "
"    {"
"      \"type\": \"Stop\","
"      \"name\": \"Biryulyovo Zapadnoye\","
"      \"latitude\": 55.574371,"
"      \"longitude\": 37.6517,"
"      \"road_distances\": {\"Rossoshanskaya ulitsa\": 7500, \"Biryusinka\": 1800, \"Universam\": 2400 }"
"    },"
"    {"
"      \"type\": \"Bus\","
"      \"name\": \"256\","
"      \"stops\": [\"Biryulyovo Zapadnoye\", \"Biryusinka\", \"Universam\", \"Biryulyovo Tovarnaya\", \"Biryulyovo Passazhirskaya\", \"Biryulyovo Zapadnoye\" ],"
"      \"is_roundtrip\": true"
"    }"
"  ],"
"  \"stat_requests\": ["
"    { \"id\": 1, \"type\": \"Bus\", \"name\": \"256\" },"
"    { \"id\": 2, \"type\": \"Stop\", \"name\": \"Biryulyovo Zapadnoye\" }"
"  ]"
"}"s
    };

    std::istringstream jinput3{
"{"
"  \"render_settings\": { "
"      \"width\": 600,"
"      \"height\": 400,"
"      \"padding\": 50,"
"      \"line_width\": 14,"
"      \"color_palette\": ["
"        \"green\","
"        [255, 160, 0],"
"        \"red\""
"      ]"
"    }"
"}"s
    };

#endif

    transport::TransportCatalogue catalogue;
    renderer::MapRenderer renderer;
    transport::RequestHandler request_handler(catalogue, renderer);

#ifdef ISSTR
    transport::JsonReader jreader(catalogue, json::Load(jinput3).GetRoot(), request_handler);
#else
    transport::JsonReader jreader(catalogue, json::Load(std::cin).GetRoot(), request_handler);
#endif

    jreader.FillDataBase();
    jreader.ExecQueries();

    return 0;
}
