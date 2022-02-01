//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETPULLERBASE_H
#define __INET_PACKETPULLERBASE_H

#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IPacketPuller.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API PacketPullerBase : public PacketProcessorBase, public virtual IPacketPuller
{
  protected:
    cGate *inputGate = nullptr;
    IPassivePacketSource *provider = nullptr;

    cGate *outputGate = nullptr;
    IActivePacketSink *collector = nullptr;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual IPassivePacketSource *getProvider(cGate *gate) override { return this; }

    virtual bool supportsPacketPushing(cGate *gate) const override { return false; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return true; }

    virtual bool canPullSomePacket(cGate *gate) const override;
    virtual Packet *canPullPacket(cGate *gate) const override;

    virtual Packet *pullPacket(cGate *gate) override;
    virtual Packet *pullPacketStart(cGate *gate, bps datarate) override;
    virtual Packet *pullPacketEnd(cGate *gate) override;
    virtual Packet *pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override;

    virtual void handleCanPullPacketChanged(cGate *gate) override;
    virtual void handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
};

} // namespace queueing
} // namespace inet

#endif

