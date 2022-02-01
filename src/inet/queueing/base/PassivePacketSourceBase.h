//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PASSIVEPACKETSOURCEBASE_H
#define __INET_PASSIVEPACKETSOURCEBASE_H

#include "inet/queueing/base/PacketSourceBase.h"
#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API PassivePacketSourceBase : public PacketSourceBase, public virtual IPassivePacketSource
{
  protected:
    cGate *outputGate = nullptr;
    IActivePacketSink *collector = nullptr;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual bool supportsPacketPushing(cGate *gate) const override { return false; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return outputGate == gate; }

    virtual bool canPullSomePacket(cGate *gate) const override { return true; }

    virtual Packet *pullPacketStart(cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketEnd(cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Invalid operation"); }
};

} // namespace queueing
} // namespace inet

#endif

