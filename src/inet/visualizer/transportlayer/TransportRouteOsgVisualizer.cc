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

#include "inet/visualizer/transportlayer/TransportRouteOsgVisualizer.h"

#ifdef WITH_ETHERNET
#include "inet/linklayer/ethernet/switch/MACRelayUnit.h"
#endif

#ifdef WITH_IEEE8021D
#include "inet/linklayer/ieee8021d/relay/Ieee8021dRelay.h"
#endif

#ifdef WITH_TCP_INET
#include "inet/transportlayer/tcp/TCP.h"
#endif

#ifdef WITH_UDP
#include "inet/transportlayer/udp/UDP.h"
#endif

namespace inet {

namespace visualizer {

Define_Module(TransportRouteOsgVisualizer);

bool TransportRouteOsgVisualizer::isPathStart(cModule *module) const
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

bool TransportRouteOsgVisualizer::isPathEnd(cModule *module) const
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

bool TransportRouteOsgVisualizer::isPathElement(cModule *module) const
{
#ifdef WITH_ETHERNET
    if (dynamic_cast<MACRelayUnit *>(module) != nullptr)
        return true;
#endif

#ifdef WITH_IEEE8021D
    if (dynamic_cast<Ieee8021dRelay *>(module) != nullptr)
        return true;
#endif

    return false;
}

} // namespace visualizer

} // namespace inet

