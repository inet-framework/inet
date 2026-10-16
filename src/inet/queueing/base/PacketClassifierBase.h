//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETCLASSIFIERBASE_H
#define __INET_PACKETCLASSIFIERBASE_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/common/ActivePacketSinkRef.h"
#include "inet/queueing/common/ActivePacketSourceRef.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/queueing/common/PassivePacketSourceRef.h"
#include "inet/queueing/contract/IPacketSink.h"
#include "inet/queueing/contract/IPacketSource.h"

namespace inet {
namespace queueing {

class INET_API PacketClassifierBase : public PacketProcessorBase, public TransparentProtocolRegistrationListener, public virtual IPacketSink, public virtual IPacketSource
{
  protected:
    bool reverseOrder = false;

    cGate *inputGate = nullptr;
    ActivePacketSourceRef producer;
    PassivePacketSourceRef provider;

    std::vector<cGate *> outputGates;
    std::vector<PassivePacketSinkRef> consumers;
    std::vector<ActivePacketSinkRef> collectors;

    int inProgressStreamId = -1;
    int inProgressGateIndex = -1;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void mapRegistrationForwardingGates(cGate *gate, std::function<void(cGate *)> f) override;

    virtual size_t getOutputGateIndex(size_t i) const { return reverseOrder ? outputGates.size() - i - 1 : i; }
    /**
     * Returns the index of the output gate for the packet. This function is a
     * query, so it must not change the state of the module. The base class
     * calls it from queries and from operations that move the packet.
     */
    virtual int classifyPacket(Packet *packet) const = 0;
    virtual int callClassifyPacket(Packet *packet) const;
    /**
     * Updates the state of the module for a packet that goes through the
     * output gate with the given index. The base class calls this function one
     * time for each packet, and one time for each packet stream, when the
     * module holds the packet and before the packet leaves the module.
     */
    virtual void handlePacketClassified(Packet *packet, int index) {}
    virtual int classifyPulledPacket(Packet *packet, const cGate *gate);

    virtual bool isStreamingPacket() const { return inProgressStreamId != -1; }
    virtual void startPacketStreaming(Packet *packet);
    virtual void endPacketStreaming(Packet *packet);
    virtual void checkPacketStreaming(Packet *packet);

  public:
    virtual IPassivePacketSource *getProvider(const cGate *gate) override { return provider; }
    virtual IPassivePacketSink *getConsumer(const cGate *gate) override { return consumers[gate->getIndex()]; }

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
    virtual Packet *canPullPacket(const cGate *gate, const IPacketFilterFunction *filter) const override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacket(const cGate *gate, const IPacketFilterFunction *filter) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketStart(const cGate *gate, bps datarate) override;
    virtual Packet *pullPacketEnd(const cGate *gate) override;
    virtual Packet *pullPacketProgress(const cGate *gate, bps datarate, b position, b extraProcessableLength) override;

    virtual void handleCanPullPacketChanged(const cGate *gate) override;
    virtual void handlePullPacketProcessed(Packet *packet, const cGate *gate, bool successful) override { throw cRuntimeError("TODO"); }
};

} // namespace queueing
} // namespace inet

#endif

