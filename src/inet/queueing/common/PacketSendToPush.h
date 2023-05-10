//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETSENDTOPUSH_H
#define __INET_PACKETSENDTOPUSH_H

#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/queueing/contract/IActivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API PacketSendToPush : public PacketProcessorBase, public IActivePacketSource
{
  protected:
    cGate *outputGate = nullptr;
    PassivePacketSinkRef consumer;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

  public:
    virtual IPassivePacketSink *getConsumer(const cGate *gate) override { return consumer; }

    virtual bool supportsPacketPushing(const cGate *gate) const override { return outputGate == gate; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return false; }

    virtual void handleCanPushPacketChanged(const cGate *gate) override {}
    virtual void handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful) override {}
};

} // namespace queueing
} // namespace inet

#endif

