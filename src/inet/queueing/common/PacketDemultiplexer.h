//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETDEMULTIPLEXER_H
#define __INET_PACKETDEMULTIPLEXER_H

#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API PacketDemultiplexer : public PacketProcessorBase, public virtual IActivePacketSink, public virtual IPassivePacketSource
{
  protected:
    cGate *inputGate = nullptr;
    IPassivePacketSource *provider = nullptr;

    std::vector<cGate *> outputGates;
    std::vector<IActivePacketSink *> collectors;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual IPassivePacketSource *getProvider(cGate *gate) override { return provider; }

    virtual bool supportsPacketPushing(cGate *gate) const override { return false; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return true; }

    virtual bool canPullSomePacket(cGate *gate) const override { return provider->canPullSomePacket(inputGate->getPathStartGate()); }
    virtual Packet *canPullPacket(cGate *gate) const override { return provider->canPullPacket(inputGate->getPathStartGate()); }

    virtual Packet *pullPacket(cGate *gate) override;

    virtual Packet *pullPacketStart(cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketEnd(cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength) override { throw cRuntimeError("Invalid operation"); }

    virtual void handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
    virtual void handleCanPullPacketChanged(cGate *gate) override;
};

} // namespace queueing
} // namespace inet

#endif

