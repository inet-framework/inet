//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETFLOWBASE_H
#define __INET_PACKETFLOWBASE_H

#include "inet/common/ModuleRef.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/common/ActivePacketSinkRef.h"
#include "inet/queueing/common/ActivePacketSourceRef.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/queueing/common/PassivePacketSourceRef.h"
#include "inet/queueing/contract/IPacketCollection.h"
#include "inet/queueing/contract/IPacketFlow.h"

namespace inet {
namespace queueing {

class INET_API PacketFlowBase : public PacketProcessorBase, public virtual IPacketFlow, public virtual IPacketCollection
{
  protected:
    cGate *inputGate = nullptr;
    ActivePacketSourceRef producer;
    PassivePacketSourceRef provider;
    ModuleRef<IPacketCollection> collection;

    cGate *outputGate = nullptr;
    PassivePacketSinkRef consumer;
    ActivePacketSinkRef collector;

    int inProgressStreamId = -1;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void processPacket(Packet *packet) = 0;

    virtual bool isStreamingPacket() const { return inProgressStreamId != -1; }
    virtual void startPacketStreaming(Packet *packet);
    virtual void endPacketStreaming(Packet *packet);
    virtual void checkPacketStreaming(Packet *packet);

  public:
    virtual IPassivePacketSink *getConsumer(const cGate *gate) override { return this; }
    virtual IPassivePacketSource *getProvider(const cGate *gate) override { return this; }

    virtual bool supportsPacketPushing(const cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return true; }
    virtual bool supportsPacketStreaming(const cGate *gate) const override { return true; }

    virtual bool canPushSomePacket(const cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override;

    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override;
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override;

    virtual void handleCanPushPacketChanged(const cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;

    virtual bool canPullSomePacket(const cGate *gate) const override;
    virtual Packet *canPullPacket(const cGate *gate) const override;

    virtual Packet *pullPacket(const cGate *gate) override;
    virtual Packet *pullPacketStart(const cGate *gate, bps datarate) override;
    virtual Packet *pullPacketEnd(const cGate *gate) override;
    virtual Packet *pullPacketProgress(const cGate *gate, bps datarate, b position, b extraProcessableLength) override;

    virtual void handleCanPullPacketChanged(const cGate *gate) override;
    virtual void handlePullPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;

    virtual int getMaxNumPackets() const override { return collection->getMaxNumPackets(); }
    virtual int getNumPackets() const override { return collection->getNumPackets(); }
    virtual b getMaxTotalLength() const override { return collection->getMaxTotalLength(); }
    virtual b getTotalLength() const override { return collection->getTotalLength(); }
    virtual Packet *getPacket(int index) const override { return collection->getPacket(index); }
    virtual bool isEmpty() const override { return collection->isEmpty(); }
    virtual void removePacket(Packet *packet) override { collection->removePacket(packet); }
    virtual void removeAllPackets() override { collection->removeAllPackets(); }
};

} // namespace queueing
} // namespace inet

#endif

