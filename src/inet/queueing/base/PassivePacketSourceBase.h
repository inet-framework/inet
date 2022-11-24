//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PASSIVEPACKETSOURCEBASE_H
#define __INET_PASSIVEPACKETSOURCEBASE_H

#include "inet/queueing/base/PacketSourceBase.h"
#include "inet/queueing/base/PassivePacketSourceMixin.h"
#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API PassivePacketSourceBase : public PassivePacketSourceMixin<PacketSourceBase>
{
  protected:
    cGate *outputGate = nullptr;
    IActivePacketSink *collector = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual Packet *handlePullPacketStart(cGate *gate, bps datarate) override { throw cRuntimeError("Illegal operation: packet streaming is not supported"); }
    virtual Packet *handlePullPacketEnd(cGate *gate) override { throw cRuntimeError("Illegal operation: packet streaming is not supported"); }
    virtual Packet *handlePullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Illegal operation: packet streaming is not supported"); }

  public:
    virtual bool supportsPacketPushing(cGate *gate) const override { return false; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return outputGate == gate; }

    virtual bool canPullSomePacket(cGate *gate) const override { return true; }
};

} // namespace queueing
} // namespace inet

#endif

