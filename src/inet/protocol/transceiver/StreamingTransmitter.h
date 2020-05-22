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

#ifndef __INET_STREAMINGTRANSMITTER_H
#define __INET_STREAMINGTRANSMITTER_H

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalMixin.h"
#include "inet/protocol/transceiver/base/PacketTransmitterBase.h"

namespace inet {

using namespace inet::queueing;
using namespace inet::physicallayer;

class INET_API StreamingTransmitter : public OperationalMixin<PacketTransmitterBase>
{
  protected:
    cPar *dataratePar = nullptr;
    bps datarate = bps(NaN);

    simtime_t txStartTime = -1;
    cMessage *txEndTimer = nullptr;
    Packet *txPacket = nullptr;
    Signal *txSignal = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;

    virtual void startTx(Packet *packet);
    virtual void endTx();
    virtual void abortTx();

    virtual simclocktime_t calculateDuration(const Packet *packet) const override;
    virtual void scheduleTxEndTimer(Signal *signal);

    // for lifecycle:
    virtual bool isInitializeStage(int stage) override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    StreamingTransmitter() { }
    virtual ~StreamingTransmitter();

    virtual bool supportsPacketStreaming(cGate *gate) const override { return true; }

    virtual bool canPushSomePacket(cGate *gate) const override { return !txEndTimer->isScheduled(); }
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return canPushSomePacket(gate); }
    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override;
    virtual b getPushPacketProcessedLength(Packet *packet, cGate *gate) override;
};

} // namespace inet

#endif // ifndef __INET_STREAMINGTRANSMITTER_H

