//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETSENDTOPUSH_H
#define __INET_PACKETSENDTOPUSH_H

#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API PacketSendToPush : public PacketProcessorBase, public IActivePacketSource
{
  protected:
    cGate *outputGate = nullptr;
    IPassivePacketSink *consumer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

  public:
    virtual IPassivePacketSink *getConsumer(cGate *gate) override { return consumer; }

    virtual bool supportsPacketPushing(cGate *gate) const override { return outputGate == gate; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }

    virtual void handleCanPushPacketChanged(cGate *gate) override {}
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override {}
};

} // namespace queueing
} // namespace inet

#endif

