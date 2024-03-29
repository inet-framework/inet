//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETPULLERBASE_H
#define __INET_PACKETPULLERBASE_H

#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/common/ActivePacketSinkRef.h"
#include "inet/queueing/common/PassivePacketSourceRef.h"
#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IPacketPuller.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API PacketPullerBase : public PacketProcessorBase, public virtual IPacketPuller
{
  protected:
    cGate *inputGate = nullptr;
    PassivePacketSourceRef provider;

    cGate *outputGate = nullptr;
    ActivePacketSinkRef collector;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual IPassivePacketSource *getProvider(const cGate *gate) override { return this; }

    virtual bool supportsPacketPushing(const cGate *gate) const override { return false; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return true; }

    virtual bool canPullSomePacket(const cGate *gate) const override;
    virtual Packet *canPullPacket(const cGate *gate) const override;

    virtual Packet *pullPacket(const cGate *gate) override;
    virtual Packet *pullPacketStart(const cGate *gate, bps datarate) override;
    virtual Packet *pullPacketEnd(const cGate *gate) override;
    virtual Packet *pullPacketProgress(const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override;

    virtual void handleCanPullPacketChanged(const cGate *gate) override;
    virtual void handlePullPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;
};

} // namespace queueing
} // namespace inet

#endif

