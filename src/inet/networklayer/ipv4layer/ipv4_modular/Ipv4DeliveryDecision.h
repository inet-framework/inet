//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV4ROUTINGDECISION_H
#define __INET_IPV4ROUTINGDECISION_H

#include "inet/common/INETDefs.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"
#include "inet/queueing/contract/IPacketPusher.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {

class INET_API Ipv4DeliveryDecision : public queueing::PacketProcessorBase, public virtual queueing::IPacketPusher
{
  protected:
    ModuleRefByPar<IIpv4RoutingTable> rt;
    ModuleRefByPar<IInterfaceTable> ift;
    cPatternMatcher directBroadcastInterfaceMatcher;

    cGate *inputGate = nullptr;
    IActivePacketSource *producer = nullptr;

    cGate *localOutputGate = nullptr;
    IPassivePacketSink *localConsumer = nullptr;

    cGate *forwardOutputGate = nullptr;
    IPassivePacketSink *forwardConsumer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    void deliverLocally(Packet *packet) { pushOrSendPacket(packet, localOutputGate, localConsumer); }
    void fragmentPostRouting(Packet *packet) { pushOrSendPacket(packet, forwardOutputGate, forwardConsumer); }
    void routeUnicastPacket(Packet *packet) { pushOrSendPacket(packet, forwardOutputGate, forwardConsumer); }
    void forwardMulticastPacket(Packet *packet) { pushOrSendPacket(packet, forwardOutputGate, forwardConsumer); }

  public:
    virtual IPassivePacketSink *getConsumer(cGate *gate) override { return this; }

    virtual bool supportsPacketPushing(cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }

    virtual bool canPushSomePacket(cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override;

    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override;
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override;

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
};

} // namespace inet

#endif
