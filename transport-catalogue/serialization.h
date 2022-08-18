#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "graph.h"
#include "router.h"

#include <transport_catalogue.pb.h>
#include <iostream>
#include <fstream>
#include <vector>

namespace transport {

class TransportCatalogue;
class TransportRouter;

struct Serial {
    static bool BaseSave(std::string fname, TransportCatalogue&,
                         renderer::RenderSettings&, TransportRouter&);
    static bool BaseLoad(std::string fname, TransportCatalogue&,
                         renderer::RenderSettings&, TransportRouter&);
};

} //namespace transport
