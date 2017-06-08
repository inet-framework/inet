//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/visualizer/transportlayer/TransportRouteCanvasVisualizer.h"

#ifdef WITH_ETHERNET
#include "inet/linklayer/ethernet/switch/MACRelayUnit.h"
#endif

#ifdef WITH_IEEE8021D
#include "inet/linklayer/ieee8021d/relay/Ieee8021dRelay.h"
#endif

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IPv4.h"
#endif

#ifdef WITH_TCP_INET
#include "inet/transportlayer/tcp/TCP.h"
#endif

#ifdef WITH_UDP
#include "inet/transportlayer/udp/UDP.h"
#endif

namespace inet {

namespace visualizer {

Define_Module(TransportRouteCanvasVisualizer);

bool TransportRouteCanvasVisualizer::isPathStart(cModule *module) const
{
#ifdef WITH_UDP
    if (dynamic_cast<UDP *>(module) != nullptr)
        return true;
#endif

#ifdef WITH_TCP_INET
    if (dynamic_cast<tcp::TCP *>(module) != nullptr)
        return true;
#endif

    return false;
}

bool TransportRouteCanvasVisualizer::isPathEnd(cModule *module) const
{
#ifdef WITH_UDP
    if (dynamic_cast<UDP *>(module) != nullptr)
        return true;
#endif

#ifdef WITH_TCP_INET
    if (dynamic_cast<tcp::TCP *>(module) != nullptr)
        return true;
#endif

    return false;
}

bool TransportRouteCanvasVisualizer::isPathElement(cModule *module) const
{
#ifdef WITH_ETHERNET
    if (dynamic_cast<MACRelayUnit *>(module) != nullptr)
        return true;
#endif

#ifdef WITH_IEEE8021D
    if (dynamic_cast<Ieee8021dRelay *>(module) != nullptr)
        return true;
#endif

#ifdef WITH_IPv4
    if (dynamic_cast<IPv4 *>(module) != nullptr)
        return true;
#endif

    return false;
}

const PathCanvasVisualizerBase::PathVisualization *TransportRouteCanvasVisualizer::createPathVisualization(const std::vector<int>& path, cPacket *packet) const
{
    auto pathVisualization = static_cast<const PathCanvasVisualization *>(PathCanvasVisualizerBase::createPathVisualization(path, packet));
    pathVisualization->figure->setTags((std::string("transport_route ") + tags).c_str());
    pathVisualization->figure->setTooltip("This polyline arrow represents a recently active transport route between two network nodes");
    pathVisualization->shiftPriority = 4;
    return pathVisualization;
}

} // namespace visualizer

} // namespace inet

