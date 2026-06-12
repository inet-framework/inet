//
// Copyright (C) 2007
// Christian Bauer
// Institute of Communications and Navigation, German Aerospace Center (DLR)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

#include "inet/networklayer/ipv6tunneling/Ipv6Tunneling.h"

namespace inet {

Define_Module(Ipv6Tunneling);

void Ipv6Tunneling::initialize(int stage)
{
    OperationalBase::initialize(stage);
}

void Ipv6Tunneling::handleMessageWhenUp(cMessage *msg)
{
    // The tunnel logic moved to xMIPv6 / Ipv6RoutingTable; this module is an inert
    // placeholder and the IPv6 layer no longer sends anything to it.
    throw cRuntimeError("Ipv6Tunneling is a disabled placeholder and must not receive messages (got '%s')", msg->getName());
}

} // namespace inet
