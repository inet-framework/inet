//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETQUEUEBASE_H
#define __INET_PACKETQUEUEBASE_H

#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IPacketQueue.h"

namespace inet {
namespace queueing {

class INET_API PacketQueueBase : public PacketProcessorBase, public virtual IPacketQueue
{
  protected:
    int numPushedPackets = -1;
    int numPulledPackets = -1;
    int numRemovedPackets = -1;
    int numDroppedPackets = -1;
    int numCreatedPackets = -1;

    cGate *inputGate = nullptr;
    cGate *outputGate = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void emit(simsignal_t signal, cObject *object, cObject *details = nullptr) override;

    virtual std::string resolveDirective(char directive) const override;

  public:
    virtual bool canPullSomePacket(cGate *gate) const override { return getNumPackets() > 0; }
    virtual bool canPushSomePacket(cGate *gate) const override { return true; }

    virtual void enqueuePacket(Packet *packet) override;
    virtual Packet *dequeuePacket() override;

    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Invalid operation"); }

    virtual Packet *pullPacketStart(cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketEnd(cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength) override { throw cRuntimeError("Invalid operation"); }
};

} // namespace queueing
} // namespace inet

#endif

