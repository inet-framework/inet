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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/tracker/transportlayer/TransportPathTracker.h"

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

namespace tracker {

Define_Module(TransportPathTracker);

bool TransportPathTracker::isPathStart(cModule *module) const
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

bool TransportPathTracker::isPathEnd(cModule *module) const
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

bool TransportPathTracker::isPathElement(cModule *module) const
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

} // namespace tracker

} // namespace inet

