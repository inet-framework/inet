//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/transportlayer/TransportRouteCanvasVisualizer.h"

#ifdef INET_WITH_ETHERNET
#include "inet/linklayer/ethernet/common/MacRelayUnit.h"
#endif

#ifdef INET_WITH_IEEE8021D
#include "inet/linklayer/ieee8021d/relay/Ieee8021dRelay.h"
#endif

#ifdef INET_WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4.h"
#endif

#ifdef INET_WITH_TCP_INET
#include "inet/transportlayer/tcp/Tcp.h"
#endif

#ifdef INET_WITH_UDP
#include "inet/transportlayer/udp/Udp.h"
#endif

namespace inet {

namespace visualizer {

Define_Module(TransportRouteCanvasVisualizer);

bool TransportRouteCanvasVisualizer::isPathStart(cModule *module) const
{
#ifdef INET_WITH_UDP
    if (dynamic_cast<Udp *>(module) != nullptr)
        return true;
#endif

#ifdef INET_WITH_TCP_INET
    if (dynamic_cast<tcp::Tcp *>(module) != nullptr)
        return true;
#endif

    return false;
}

bool TransportRouteCanvasVisualizer::isPathEnd(cModule *module) const
{
#ifdef INET_WITH_UDP
    if (dynamic_cast<Udp *>(module) != nullptr)
        return true;
#endif

#ifdef INET_WITH_TCP_INET
    if (dynamic_cast<tcp::Tcp *>(module) != nullptr)
        return true;
#endif

    return false;
}

bool TransportRouteCanvasVisualizer::isPathElement(cModule *module) const
{
#ifdef INET_WITH_ETHERNET
    if (dynamic_cast<MacRelayUnit *>(module) != nullptr)
        return true;
#endif

#ifdef INET_WITH_IEEE8021D
    if (dynamic_cast<Ieee8021dRelay *>(module) != nullptr)
        return true;
#endif

#ifdef INET_WITH_IPv4
    if (dynamic_cast<Ipv4 *>(module) != nullptr)
        return true;
#endif

    return false;
}

const PathCanvasVisualizerBase::PathVisualization *TransportRouteCanvasVisualizer::createPathVisualization(const char *label, const std::vector<int>& path, cPacket *packet) const
{
    auto pathVisualization = static_cast<const PathCanvasVisualization *>(PathCanvasVisualizerBase::createPathVisualization(label, path, packet));
    pathVisualization->figure->setTags((std::string("transport_route ") + tags).c_str());
    pathVisualization->figure->setTooltip("This polyline arrow represents a recently active transport route between two network nodes");
    pathVisualization->shiftPriority = 4;
    return pathVisualization;
}

} // namespace visualizer

} // namespace inet

