//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;

    virtual bool isInitializeStage(int stage) override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;

    virtual Signal *encodePacket(Packet *packet) const;

    virtual void sendSignalStart(Signal *signal, int transmissionId);
    virtual void sendSignalProgress(Signal *signal, int transmissionId, b bitPosition, clocktime_t timePosition);
    virtual void sendSignalEnd(Signal *signal, int transmissionId);

    virtual clocktime_t calculateDuration(const Packet *packet) const;
    virtual bool isTransmitting() const { return txSignal != nullptr; }

  public:
    virtual ~PacketTransmitterBase();

    virtual bool supportsPacketPushing(cGate *gate) const override { return inputGate == gate; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }

    virtual bool canPushSomePacket(cGate *gate) const override { return !txEndTimer->isScheduled(); }
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return canPushSomePacket(gate); }

    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Invalid operation"); }

};

} // namespace inet

#endif

