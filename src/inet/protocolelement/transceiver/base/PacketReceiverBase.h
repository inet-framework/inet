//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETRECEIVERBASE_H
#define __INET_PACKETRECEIVERBASE_H

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalMixin.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/common/Signal.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"

namespace inet {

using namespace inet::queueing;
using namespace inet::physicallayer;

extern template class OperationalMixin<PacketProcessorBase>;

class INET_API PacketReceiverBase : public OperationalMixin<PacketProcessorBase>, public virtual IActivePacketSource
{
  protected:
    const NetworkInterface *networkInterface = nullptr;

    bps rxDatarate = bps(NaN);

    cGate *inputGate = nullptr;
    cGate *outputGate = nullptr;
    IPassivePacketSink *consumer = nullptr;

    Signal *rxSignal = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void handleMessageWhenDown(cMessage *message) override;
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual Packet *decodePacket(Signal *signal) const;

  public:
    virtual ~PacketReceiverBase();

    virtual IPassivePacketSink *getConsumer(cGate *gate) override { return consumer; }

    virtual bool supportsPacketPushing(cGate *gate) const override { return gate == outputGate; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }

    virtual void handleCanPushPacketChanged(cGate *gate) override {}
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override {}
};

} // namespace inet

#endif

