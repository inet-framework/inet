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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_PACKETRECEIVERBASE_H
#define __INET_PACKETRECEIVERBASE_H

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalMixin.h"
#include "inet/physicallayer/common/packetlevel/Signal.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"

namespace inet {

using namespace inet::queueing;
using namespace inet::physicallayer;

class INET_API PacketReceiverBase : public OperationalMixin<PacketProcessorBase>, public virtual IActivePacketSource
{
  protected:
    bps datarate = bps(NaN);

    cGate *inputGate = nullptr;
    cGate *outputGate = nullptr;
    IPassivePacketSink *consumer = nullptr;

    Signal *rxSignal = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual bool isInitializeStage(int stage) override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
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

    virtual void handleCanPushPacketChanged(cGate *gate) override { }
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override { }
};

} // namespace inet

#endif // ifndef __INET_PACKETRECEIVERBASE_H

