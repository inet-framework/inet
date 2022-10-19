//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV4LOCALOUT_H
#define __INET_IPV4LOCALOUT_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4layer/icmp/Icmp.h"
#include "inet/networklayer/ipv4layer/routingtable/IIpv4RoutingTable.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

class INET_API Ipv4LocalOut : public queueing::PacketPusherBase
{
  protected:
    ModuleRefByPar<Icmp> icmp;
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<IIpv4RoutingTable> rt;
    bool limitedBroadcast = false;

  protected:
    virtual void initialize(int stage) override;
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    const NetworkInterface *getSourceInterface(Packet *packet);
    const NetworkInterface *getDestInterface(Packet *packet);
    Ipv4Address getNextHop(Packet *packet);
    void datagramLocalOut(Packet *packet);
    const NetworkInterface *determineOutgoingInterfaceForMulticastDatagram(const Ptr<const Ipv4Header>& ipv4Header, const NetworkInterface *multicastIFOption);
    void routeMulticastPacket(Packet *packet);
    void routeUnicastPacket(Packet *packet);
    void routeLocalBroadcastPacket(Packet *packet);
    void setHeaderSrcAddressByDestInterface(Packet *packet);
};

} // namespace inet

#endif
