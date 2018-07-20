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

#include "inet/visualizer/networklayer/NetworkRouteOsgVisualizer.h"

#ifdef WITH_ETHERNET
#include "inet/linklayer/ethernet/switch/MacRelayUnit.h"
#endif

#ifdef WITH_IEEE8021D
#include "inet/linklayer/ieee8021d/relay/Ieee8021dRelay.h"
#endif

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4.h"
#endif

namespace inet {

namespace visualizer {

Define_Module(NetworkRouteOsgVisualizer);

bool NetworkRouteOsgVisualizer::isPathStart(cModule *module) const
{
#ifdef WITH_IPv4
    if (dynamic_cast<Ipv4 *>(module) != nullptr)
        return true;
#endif

    return false;
}

bool NetworkRouteOsgVisualizer::isPathEnd(cModule *module) const
{
#ifdef WITH_IPv4
    if (dynamic_cast<Ipv4 *>(module) != nullptr)
        return true;
#endif

    return false;
}

bool NetworkRouteOsgVisualizer::isPathElement(cModule *module) const
{
#ifdef WITH_ETHERNET
    if (dynamic_cast<MacRelayUnit *>(module) != nullptr)
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

