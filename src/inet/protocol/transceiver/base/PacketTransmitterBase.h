//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_PACKETTRANSMITTERBASE_H
#define __INET_PACKETTRANSMITTERBASE_H

#include "inet/common/base/ClockUsingModuleMixin.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalMixin.h"
#include "inet/physicallayer/common/packetlevel/Signal.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {

using namespace inet::queueing;
using namespace inet::physicallayer;

class INET_API PacketTransmitterBase : public ClockUsingModuleMixin<OperationalMixin<PacketProcessorBase>>, public virtual IPassivePacketSink
{
  protected:
    cPar *dataratePar = nullptr;
    bps datarate = bps(NaN);

    cGate *inputGate = nullptr;
    cGate *outputGate = nullptr;
    IActivePacketSource *producer = nullptr;

    int txId = -1;
    Signal *txSignal = nullptr;
    cMessage *txEndTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;

    virtual bool isInitializeStage(int stage) override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;

    virtual Signal *encodePacket(Packet *packet) const;

    virtual void sendPacketStart(Signal *signal);
    virtual void sendPacketProgress(Signal *signal, b bitPosition, clocktime_t timePosition);
    virtual void sendPacketEnd(Signal *signal);

    virtual clocktime_t calculateDuration(const Packet *packet) const;
    virtual bool isTransmitting() const { return txSignal != nullptr; }

  public:
    virtual ~PacketTransmitterBase();

    virtual bool supportsPacketPushing(cGate *gate) const override { return inputGate == gate; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }

    virtual bool canPushSomePacket(cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return true; }

    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketEnd(Packet *packet, cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Invalid operation"); }

    virtual b getPushPacketProcessedLength(Packet *packet, cGate *gate) override { throw cRuntimeError("Invalid operation"); }
};

} // namespace inet

#endif // ifndef __INET_PACKETTRANSMITTERBASE_H

