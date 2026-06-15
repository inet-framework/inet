//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/networklayer/NetworkRouteVsgVisualizer.h"

#ifdef INET_WITH_ETHERNET
#include "inet/linklayer/ethernet/common/MacRelayUnit.h"
#endif

#ifdef INET_WITH_IEEE8021D
#include "inet/linklayer/ieee8021d/relay/Ieee8021dRelay.h"
#endif

#ifdef INET_WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4.h"
#endif

namespace inet {

namespace visualizer {

Define_Module(NetworkRouteVsgVisualizer);

bool NetworkRouteVsgVisualizer::isPathStart(cModule *module) const
{
#ifdef INET_WITH_IPv4
    if (dynamic_cast<Ipv4 *>(module) != nullptr)
        return true;
#endif

    return false;
}

bool NetworkRouteVsgVisualizer::isPathEnd(cModule *module) const
{
#ifdef INET_WITH_IPv4
    if (dynamic_cast<Ipv4 *>(module) != nullptr)
        return true;
#endif

    return false;
}

bool NetworkRouteVsgVisualizer::isPathElement(cModule *module) const
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
