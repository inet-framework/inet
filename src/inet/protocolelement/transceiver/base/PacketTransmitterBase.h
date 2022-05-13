//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETTRANSMITTERBASE_H
#define __INET_PACKETTRANSMITTERBASE_H

#include "inet/common/ModuleRef.h"
#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalMixin.h"
#include "inet/physicallayer/common/Signal.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {

using namespace inet::queueing;
using namespace inet::physicallayer;

extern template class ClockUserModuleMixin<OperationalMixin<PacketProcessorBase>>;

class INET_API PacketTransmitterBase : public ClockUserModuleMixin<OperationalMixin<PacketProcessorBase>>, public virtual IPassivePacketSink
{
  protected:
    cPar *dataratePar = nullptr;

    cGate *inputGate = nullptr;
    cGate *outputGate = nullptr;
    ModuleRef<IActivePacketSource> producer;

    bps txDatarate = bps(NaN);
    Signal *txSignal = nullptr;
    ClockEvent *txEndTimer = nullptr;

    simtime_t txStartTime = -1;
    clocktime_t txStartClockTime = -1;
    clocktime_t txDurationClockTime = -1;

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessageWhenUp(cMessage *message) override;

    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;

    virtual Signal *encodePacket(Packet *packet);
    virtual void prepareSignal(Signal *signal);

    virtual void sendSignalStart(Signal *signal, int transmissionId);
    virtual void sendSignalProgress(Signal *signal, int transmissionId, b bitPosition, clocktime_t timePosition);
    virtual void sendSignalEnd(Signal *signal, int transmissionId);

    virtual clocktime_t calculateClockTimeDuration(const Packet *packet) const;
    virtual simtime_t calculateDuration(clocktime_t clockTimeDuration) const;
    virtual bool isTransmitting() const { return txSignal != nullptr; }

  public:
    virtual ~PacketTransmitterBase();

    virtual bool supportsPacketPushing(cGate *gate) const override { return inputGate == gate; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }

    virtual bool canPushSomePacket(cGate *gate) const override { return !txEndTimer->isScheduled(); } // TODO: add hasCarrier
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return canPushSomePacket(gate); }

    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Invalid operation"); }
};

} // namespace inet

#endif

