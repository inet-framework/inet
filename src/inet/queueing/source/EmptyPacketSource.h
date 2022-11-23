//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_EMPTYPACKETSOURCE_H
#define __INET_EMPTYPACKETSOURCE_H

#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IPacketSource.h"

namespace inet {
namespace queueing {

class INET_API EmptyPacketSource : public PacketProcessorBase, public virtual IPacketSource
{
  protected:
    cGate *outputGate = nullptr;
    IPassivePacketSink *consumer = nullptr;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual IPassivePacketSink *getConsumer(cGate *gate) override { return consumer; }

    virtual bool supportsPacketPushing(cGate *gate) const override { return outputGate == gate; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return outputGate == gate; }

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;

    virtual bool canPullSomePacket(cGate *gate) const override { return false; }
    virtual Packet *canPullPacket(cGate *gate) const override { return nullptr; }

    virtual Packet *pullPacket(cGate *gate) override { throw cRuntimeError("Illegal operation: packet source is empty"); }
    virtual Packet *pullPacketStart(cGate *gate, bps datarate)  override { throw cRuntimeError("Illegal operation: packet source is empty"); }
    virtual Packet *pullPacketEnd(cGate *gate)  override { throw cRuntimeError("Illegal operation: packet source is empty"); }
    virtual Packet *pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength)  override { throw cRuntimeError("Illegal operation: packet source is empty"); }
};

} // namespace queueing
} // namespace inet

#endif

