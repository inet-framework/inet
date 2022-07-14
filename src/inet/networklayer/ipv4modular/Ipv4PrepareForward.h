//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV4PREPAREFORWARD_H
#define __INET_IPV4PREPAREFORWARD_H

#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/Icmp.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

class INET_API Ipv4PrepareForward : public queueing::PacketPusherBase
{
  protected:
    ModuleRefByPar<Icmp> icmp;
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<IIpv4RoutingTable> rt;
  protected:
    virtual void initialize(int stage) override;
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    void routeUnicastPacket(Packet *packet);
    void forwardMulticastPacket(Packet *packet);
};

} // namespace inet

#endif
