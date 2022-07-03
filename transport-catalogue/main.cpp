#include <iostream>
#include <sstream>

#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_router.h"

//#define ISSTR ;

using namespace std::literals;

int main()
{

#ifdef ISSTR

    std::istringstream sin{
"{"
"    \"base_requests\": ["
"        {"
"            \"is_roundtrip\": true,"
"            \"name\": \"297\","
"            \"stops\": ["
"                \"Biryulyovo Zapadnoye\","
"                \"Biryulyovo Tovarnaya\","
"                \"Universam\","
"                \"Biryulyovo Zapadnoye\""
"            ],"
"            \"type\": \"Bus\""
"        },"
"        {"
"            \"is_roundtrip\": false,"
"            \"name\": \"635\","
"            \"stops\": ["
"                \"Biryulyovo Tovarnaya\","
"                \"Universam\","
"                \"Prazhskaya\""
"            ],"
"            \"type\": \"Bus\""
"        },"
"        {"
"            \"latitude\": 55.574371,"
"            \"longitude\": 37.6517,"
"            \"name\": \"Biryulyovo Zapadnoye\","
"            \"road_distances\": {"
"                \"Biryulyovo Tovarnaya\": 2600"
"            },"
"            \"type\": \"Stop\""
"        },"
"        {"
"            \"latitude\": 55.587655,"
"            \"longitude\": 37.645687,"
"            \"name\": \"Universam\","
"            \"road_distances\": {"
"                \"Biryulyovo Tovarnaya\": 1380,"
"                \"Biryulyovo Zapadnoye\": 2500,"
"                \"Prazhskaya\": 4650"
"            },"
"            \"type\": \"Stop\""
"        },"
"        {"
"            \"latitude\": 55.592028,"
"            \"longitude\": 37.653656,"
"            \"name\": \"Biryulyovo Tovarnaya\","
"            \"road_distances\": {"
"                \"Universam\": 890"
"            },"
"            \"type\": \"Stop\""
"        },"
"        {"
"            \"latitude\": 55.611717,"
"            \"longitude\": 37.603938,"
"            \"name\": \"Prazhskaya\","
"            \"road_distances\": {},"
"            \"type\": \"Stop\""
"        }"
"    ],"
"    \"routing_settings\": {"
"        \"bus_velocity\": 40,"
"        \"bus_wait_time\": 6"
"    },"
"    \"stat_requests\": ["
"        {"
"            \"id\": 1,"
"            \"name\": \"297\","
"            \"type\": \"Bus\""
"        },"
"        {"
"            \"id\": 2,"
"            \"name\": \"635\","
"            \"type\": \"Bus\""
"        },"
"        {"
"            \"id\": 3,"
"            \"name\": \"Universam\","
"            \"type\": \"Stop\""
"        },"
"        {"
"            \"from\": \"Biryulyovo Zapadnoye\","
"            \"id\": 4,"
"            \"to\": \"Universam\","
"            \"type\": \"Route\""
"        },"
"        {"
"            \"from\": \"Biryulyovo Zapadnoye\","
"            \"id\": 5,"
"            \"to\": \"Prazhskaya\","
"            \"type\": \"Route\""
"        }"
"    ]"
"}"
""s
};

#endif

    transport::TransportCatalogue catalogue;
    renderer::MapRenderer renderer;
    transport::TransportRouter router(catalogue);
    transport::RequestHandler request_handler(catalogue, renderer, router);

#ifdef ISSTR
    transport::JsonReader jreader(catalogue, json::Load(sin).GetRoot(), request_handler);
#else
    transport::JsonReader jreader(catalogue, json::Load(std::cin).GetRoot(), request_handler);
#endif

    jreader.FillDataBase();
    jreader.ExecQueries();

    return 0;
}
