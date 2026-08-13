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
     * Returns the index of the output gate the packet is classified to, or -1
     * if no existing output gate suits the packet. Classification is a query
     * and must be free of side effects: the capacity checks (canPushPacket(),
     * canPullPacket()) classify the same packet as its eventual delivery, and
     * the pull path classifies it more than once.
     */
    virtual int classifyPacket(Packet *packet) = 0;
    virtual int callClassifyPacket(Packet *packet) const;

    /**
     * Called when a packet being pushed is classified to no existing output
     * gate. This is where side effects of taking such a packet belong: a
     * classifier that extends itself on demand creates the new output gate
     * here and returns its index. Called from packet delivery only, never
     * from a query. The default refuses the packet with an error.
     */
    virtual int createGateForPacket(Packet *packet);

    /**
     * Returns true if createGateForPacket() would provide an output gate for
     * the packet: the query pair of createGateForPacket(), consulted by
     * canPushPacket() when classifyPacket() finds no gate.
     */
    virtual bool canCreateGateForPacket(Packet *packet) const;

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
    virtual Packet *pullPacketStart(const cGate *gate, bps datarate) override;
    virtual Packet *pullPacketEnd(const cGate *gate) override;
    virtual Packet *pullPacketProgress(const cGate *gate, bps datarate, b position, b extraProcessableLength) override;

    virtual void handleCanPullPacketChanged(const cGate *gate) override;
    virtual void handlePullPacketProcessed(Packet *packet, const cGate *gate, bool successful) override { throw cRuntimeError("TODO"); }
};

} // namespace queueing
} // namespace inet

#endif

