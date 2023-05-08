//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FULLPACKETSINK_H
#define __INET_FULLPACKETSINK_H

#include "inet/queueing/base/PacketSinkBase.h"
#include "inet/queueing/contract/IPacketSink.h"

namespace inet {
namespace queueing {

class INET_API FullPacketSink : public PacketProcessorBase, public virtual IPacketSink
{
  protected:
    cGate *inputGate = nullptr;
    IPassivePacketSource *provider = nullptr;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual IPassivePacketSource *getProvider(const cGate *gate) override { return provider; }

    virtual bool supportsPacketPushing(const cGate *gate) const override { return inputGate == gate; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return inputGate == gate; }

    virtual void handleCanPullPacketChanged(const cGate *gate) override;
    virtual void handlePullPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;

    virtual bool canPushSomePacket(const cGate *gate) const override { return false; }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return false; }

    virtual void pushPacket(Packet *packet, const cGate *gate) override { return throw cRuntimeError("Illegal operation: packet sink is full"); }
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { return throw cRuntimeError("Illegal operation: packet sink is full"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { return throw cRuntimeError("Illegal operation: packet sink is full"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { return throw cRuntimeError("Illegal operation: packet sink is full"); }
};

} // namespace queueing
} // namespace inet

#endif

