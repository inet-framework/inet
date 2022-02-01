//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/transportlayer/TransportRouteOsgVisualizer.h"

#ifdef INET_WITH_ETHERNET
#include "inet/linklayer/ethernet/common/MacRelayUnit.h"
#endif

#ifdef INET_WITH_IEEE8021D
#include "inet/linklayer/ieee8021d/relay/Ieee8021dRelay.h"
#endif

#ifdef INET_WITH_TCP_INET
#include "inet/transportlayer/tcp/Tcp.h"
#endif

#ifdef INET_WITH_UDP
#include "inet/transportlayer/udp/Udp.h"
#endif

namespace inet {

namespace visualizer {

Define_Module(TransportRouteOsgVisualizer);

bool TransportRouteOsgVisualizer::isPathStart(cModule *module) const
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

bool TransportRouteOsgVisualizer::isPathEnd(cModule *module) const
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

bool TransportRouteOsgVisualizer::isPathElement(cModule *module) const
{
#ifdef INET_WITH_ETHERNET
    if (dynamic_cast<MacRelayUnit *>(module) != nullptr)
        return true;
#endif

#ifdef INET_WITH_IEEE8021D
    if (dynamic_cast<Ieee8021dRelay *>(module) != nullptr)
        return true;
#endif

    return false;
}

} // namespace visualizer

} // namespace inet

