//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETFILTERBASE_H
#define __INET_PACKETFILTERBASE_H

#include "inet/common/ModuleRef.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IPacketFilter.h"

namespace inet {
namespace queueing {

// TODO add TransparentProtocolRegistrationListener as base class, but it would break nearly all
// simulations because protocol header checker modules are also derived from PacketFilterBase and
// this change would cause multiple protocol registrations reaching the same MessageDispatcher from
// different paths (e.g. ethernetmac service registration starts from EthernetMacHeaderInserter and
// ends up multiple times in the li MessageDispatcher module)
class INET_API PacketFilterBase : public PacketProcessorBase, public virtual IPacketFilter
{
  protected:
    bool backpressure = false;

    cGate *inputGate = nullptr;
    ModuleRef<IActivePacketSource> producer;
    ModuleRef<IPassivePacketSource> provider;

    cGate *outputGate = nullptr;
    ModuleRef<IPassivePacketSink> consumer;
    ModuleRef<IActivePacketSink> collector;

    int inProgressStreamId = -1;

    int numDroppedPackets = 0;
    b droppedTotalLength = b(-1);

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void processPacket(Packet *packet) {}

    virtual bool isStreamingPacket() const { return inProgressStreamId != -1; }
    virtual void startPacketStreaming(Packet *packet);
    virtual void endPacketStreaming(Packet *packet);
    virtual void checkPacketStreaming(Packet *packet);

    virtual void dropPacket(Packet *packet);
    virtual void dropPacket(Packet *packet, PacketDropReason reason, int limit = -1) override;

    virtual std::string resolveDirective(char directive) const override;

  public:
    virtual IPassivePacketSink *getConsumer(cGate *gate) override { return this; }
    virtual IPassivePacketSource *getProvider(cGate *gate) override { return this; }

    virtual bool supportsPacketPushing(cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return true; }
    virtual bool supportsPacketStreaming(cGate *gate) const override { return true; }

    virtual bool canPushSomePacket(cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override;

    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override;
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override;

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;

    virtual bool canPullSomePacket(cGate *gate) const override;
    virtual Packet *canPullPacket(cGate *gate) const override;

    virtual Packet *pullPacket(cGate *gate) override;

    virtual Packet *pullPacketStart(cGate *gate, bps datarate) override;
    virtual Packet *pullPacketEnd(cGate *gate) override;
    virtual Packet *pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength) override;

    virtual void handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
    virtual void handleCanPullPacketChanged(cGate *gate) override;
};

} // namespace queueing
} // namespace inet

#endif

