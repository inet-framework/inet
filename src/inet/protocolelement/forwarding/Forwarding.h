//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FORWARDING_H
#define __INET_FORWARDING_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

using namespace inet::queueing;

/**
 * A minimal hop-by-hop forwarding element. Packets carry a DestinationL3AddressHeader;
 * this element either delivers a packet locally (its destination equals this node's
 * address) or forwards it towards the next hop dictated by a static routing table.
 *
 * The routing table is a NED parameter (see the "routes" parameter) rather than being
 * hard-coded, so a single element type serves any topology and the table is a natural
 * target for external configuration/synthesis.
 */
class INET_API Forwarding : public PacketPusherBase
{
  protected:
    struct Route {
        L3Address destination;
        L3Address nextHop;
        int interface = -1; // interface id, or index into interfaceTable when one is configured
    };

    L3Address address;
    std::vector<Route> routes;
    ModuleRefByPar<IInterfaceTable> interfaceTable; // optional; when set, route interface fields are indices

  protected:
    virtual void initialize(int stage) override;
    virtual void parseRoutes(const char *routesString);

    // Returns {nextHop, interface} for a destination, or {unspecified, -1} if there is no route.
    // Kept virtual as the extension seam for a smarter routing policy.
    virtual std::pair<L3Address, int> findNextHop(const L3Address& destinationAddress) const;

    // Translates a route's interface field (an index when interfaceTable is set, otherwise a
    // literal interface id) into a concrete interface id; passes -1 through unchanged.
    virtual int resolveInterfaceId(int interface) const;

  public:
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
};

} // namespace inet

#endif

