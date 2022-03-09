//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/networklayer/NetworkRouteCanvasVisualizer.h"

#ifdef INET_WITH_PROTOCOLELEMENT
#include "inet/protocolelement/common/PacketEmitter.h"
#endif // INET_WITH_PROTOCOLELEMENT

#ifdef INET_WITH_ETHERNET
#include "inet/linklayer/ethernet/common/MacRelayUnit.h"
#endif

#ifdef INET_WITH_IEEE8021D
#include "inet/linklayer/ieee8021d/relay/Ieee8021dRelay.h"
#endif

#ifdef INET_WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4.h"
#endif

#ifdef INET_WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6.h"
#endif

namespace inet {

namespace visualizer {

Define_Module(NetworkRouteCanvasVisualizer);

bool NetworkRouteCanvasVisualizer::isPathStart(cModule *module) const
{
#ifdef INET_WITH_IPv4
    if (dynamic_cast<Ipv4 *>(module) != nullptr)
        return true;
#endif

#ifdef INET_WITH_IPv6
    if (dynamic_cast<Ipv6 *>(module) != nullptr)
        return true;
#endif

    return false;
}

bool NetworkRouteCanvasVisualizer::isPathEnd(cModule *module) const
{
#ifdef INET_WITH_IPv4
    if (dynamic_cast<Ipv4 *>(module) != nullptr)
        return true;
#endif

#ifdef INET_WITH_IPv6
    if (dynamic_cast<Ipv6 *>(module) != nullptr)
        return true;
#endif

    return false;
}

bool NetworkRouteCanvasVisualizer::isPathElement(cModule *module) const
{
    // KLUDGE: for visualizing when using the layered Ethernet model
#ifdef INET_WITH_PROTOCOLELEMENT
    if (dynamic_cast<PacketEmitter *>(module) != nullptr)
        return true;
#endif // INET_WITH_PROTOCOLELEMENT

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

#ifdef INET_WITH_IPv6
    if (dynamic_cast<Ipv6 *>(module) != nullptr)
        return true;
#endif

    return false;
}

const PathCanvasVisualizerBase::PathVisualization *NetworkRouteCanvasVisualizer::createPathVisualization(const char *label, const std::vector<int>& path, cPacket *packet) const
{
    auto pathVisualization = static_cast<const PathCanvasVisualization *>(PathCanvasVisualizerBase::createPathVisualization(label, path, packet));
    pathVisualization->figure->setTags((std::string("network_route ") + tags).c_str());
    pathVisualization->figure->setTooltip("This polyline arrow represents a recently active network route between two network nodes");
    pathVisualization->shiftPriority = 3;
    return pathVisualization;
}

} // namespace visualizer

} // namespace inet

