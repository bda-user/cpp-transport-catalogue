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

    static bool SaveCatalogue(TransportCatalogue&, transport::serial::Catalogue&);

    static bool SaveRenderSettings(renderer::RenderSettings&,
                                   transport::serial::RenderSettings&);

    static bool SaveRouter(TransportRouter&, transport::serial::Router&);

    static bool SaveGraph(TransportRouter&, transport::serial::Graph&);

    static bool SaveBase(std::string fname, TransportCatalogue&,
                         renderer::RenderSettings&, TransportRouter&);

    static bool LoadCatalogue(transport::serial::TransportCatalogue&,
                              TransportCatalogue&,
                              std::vector<transport::Stop*>&,
                              std::vector<transport::Bus*>&);

    static bool LoadRenderSettings(transport::serial::TransportCatalogue&,
                                   renderer::RenderSettings&);

    static bool LoadRouter(transport::serial::TransportCatalogue&,
                           TransportRouter&,
                           std::vector<transport::Stop*>&,
                           std::vector<transport::Bus*>&);

    static bool LoadGraph(transport::serial::TransportCatalogue&,
                          TransportRouter&);

    static bool LoadBase(std::string fname, TransportCatalogue&,
                         renderer::RenderSettings&, TransportRouter&);
};

} //namespace transport
