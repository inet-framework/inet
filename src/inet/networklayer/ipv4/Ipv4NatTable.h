//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV4NATTABLE_H
#define __INET_IPV4NATTABLE_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/ipv4/Ipv4NatEntry_m.h"

namespace inet {

class INET_API Ipv4NatTable : public cSimpleModule, public NetfilterBase::HookBase
{
  protected:
    cXMLElement *config = nullptr;
    ModuleRefByPar<INetfilter> networkProtocol;

    std::multimap<INetfilter::IHook::Type, std::pair<PacketFilter *, Ipv4NatEntry>> natEntries;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void parseConfig();
    virtual Result processPacket(Packet *packet, INetfilter::IHook::Type type);

  public:
    virtual ~Ipv4NatTable();
    virtual Result datagramPreRoutingHook(Packet *datagram) override { return processPacket(datagram, PREROUTING); }
    virtual Result datagramForwardHook(Packet *datagram) override { return processPacket(datagram, FORWARD); }
    virtual Result datagramPostRoutingHook(Packet *datagram) override { return processPacket(datagram, POSTROUTING); }
    virtual Result datagramLocalInHook(Packet *datagram) override { return processPacket(datagram, LOCALIN); }
    virtual Result datagramLocalOutHook(Packet *datagram) override { return processPacket(datagram, LOCALOUT); }
};

} // namespace inet

#endif

