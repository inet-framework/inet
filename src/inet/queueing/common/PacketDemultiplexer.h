//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETDEMULTIPLEXER_H
#define __INET_PACKETDEMULTIPLEXER_H

#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/common/ActivePacketSinkRef.h"
#include "inet/queueing/common/PassivePacketSourceRef.h"
#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API PacketDemultiplexer : public PacketProcessorBase, public virtual IActivePacketSink, public virtual IPassivePacketSource
{
  protected:
    cGate *inputGate = nullptr;
    PassivePacketSourceRef provider;

    std::vector<cGate *> outputGates;
    std::vector<ActivePacketSinkRef> collectors;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual IPassivePacketSource *getProvider(const cGate *gate) override { return provider; }

    virtual bool supportsPacketPushing(const cGate *gate) const override { return false; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return true; }

    virtual bool canPullSomePacket(const cGate *gate) const override { return provider->canPullSomePacket(provider.getReferencedGate()); }
    virtual Packet *canPullPacket(const cGate *gate) const override { return provider->canPullPacket(provider.getReferencedGate()); }

    virtual Packet *pullPacket(const cGate *gate) override;

    virtual Packet *pullPacketStart(const cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketEnd(const cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketProgress(const cGate *gate, bps datarate, b position, b extraProcessableLength) override { throw cRuntimeError("Invalid operation"); }

    virtual void handlePullPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;
    virtual void handleCanPullPacketChanged(const cGate *gate) override;
};

} // namespace queueing
} // namespace inet

#endif

