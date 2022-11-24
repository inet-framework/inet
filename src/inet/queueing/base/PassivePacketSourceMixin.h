//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PASSIVEPACKETSOURCEMIXIN_H
#define __INET_PASSIVEPACKETSOURCEMIXIN_H

#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

template <typename T>
class INET_API PassivePacketSourceMixin : public T, public virtual IPassivePacketSource
{
  protected:
    virtual Packet *handlePullPacket(cGate *gate) { throw cRuntimeError("Illegal operation: pullPacket is not supported"); }
    virtual Packet *handlePullPacketStart(cGate *gate, bps datarate) { throw cRuntimeError("Illegal operation: pullPacketStart is not supported"); }
    virtual Packet *handlePullPacketEnd(cGate *gate) { throw cRuntimeError("Illegal operation: pullPacketEnd is not supported"); }
    virtual Packet *handlePullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) { throw cRuntimeError("Illegal operation: pullPacketProgress is not supported"); }

  public:
    virtual Packet *pullPacket(cGate *gate) override {
        Enter_Method("pullPacket");
        ASSERT(gate->getOwnerModule() == this);
        ASSERT(canPullPacket(gate));
        auto packet = handlePullPacket(gate);
        ASSERT(packet != nullptr);
        return packet;
    }

    virtual Packet *pullPacketStart(cGate *gate, bps datarate) override {
        Enter_Method("pullPacketStart");
        ASSERT(gate->getOwnerModule() == this);
        ASSERT(datarate > bps(0));
        ASSERT(canPullPacket(gate));
        auto packet = handlePullPacketStart(gate, datarate);
        ASSERT(packet != nullptr);
        return packet;
    }

    virtual Packet *pullPacketEnd(cGate *gate) override {
        Enter_Method("pullPacketEnd");
        ASSERT(gate->getOwnerModule() == this);
        ASSERT(canPullPacket(gate));
        auto packet = handlePullPacketEnd(gate);
        ASSERT(packet != nullptr);
        return packet;
    }

    virtual Packet *pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override {
        Enter_Method("pullPacketProgress");
        ASSERT(gate->getOwnerModule() == this);
        ASSERT(datarate > bps(0));
        ASSERT(position > b(0));
        ASSERT(extraProcessableLength >= b(0));
        ASSERT(canPullPacket(gate));
        auto packet = handlePullPacketProgress(gate, datarate, position, extraProcessableLength);
        ASSERT(packet != nullptr);
        return packet;
    }
};

} // namespace queueing
} // namespace inet

#endif

