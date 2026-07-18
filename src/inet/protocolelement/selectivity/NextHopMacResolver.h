//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NEXTHOPMACRESOLVER_H
#define __INET_NEXTHOPMACRESOLVER_H

#include <map>

#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

/**
 * Turns a next-hop L3 address (in a NextHopAddressReq tag, set by Forwarding) into a link-layer
 * destination (a MacAddressReq tag) using a static, configured neighbour table -- a minimal static
 * "ARP". Needed on a shared medium so SendToMacAddress can address the intended next hop.
 */
class INET_API NextHopMacResolver : public PacketFlowBase
{
  protected:
    std::map<L3Address, MacAddress> neighbors;

  protected:
    virtual void initialize(int stage) override;
    virtual void parseNeighbors(const char *neighborsString);
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif
