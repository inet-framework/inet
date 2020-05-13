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

#ifndef __INET_STREAMINGRECEIVER_H
#define __INET_STREAMINGRECEIVER_H

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalMixin.h"
#include "inet/physicallayer/common/packetlevel/Signal.h"
#include "inet/protocol/transceiver/base/PacketReceiverBase.h"

namespace inet {

using namespace inet::units::values;
using namespace inet::physicallayer;

class INET_API StreamingReceiver : public OperationalMixin<PacketReceiverBase>
{
  protected:
    bps datarate = bps(NaN);

    Signal *rxSignal = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override { OperationalMixin::handleMessage(msg); }
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void handleMessageWhenDown(cMessage *message) override;

    virtual void sendToUpperLayer(Packet *packet);

    virtual void receivePacketStart(cPacket *packet, cGate *gate, double datarate) override;
    virtual void receivePacketProgress(cPacket *packet, cGate *gate, double datarate, int bitPosition, simtime_t timePosition, int extraProcessableBitLength, simtime_t extraProcessableDuration) override;
    virtual void receivePacketEnd(cPacket *packet, cGate *gate, double datarate) override;

    // for lifecycle:
    virtual bool isInitializeStage(int stage) override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    StreamingReceiver() {}
    virtual ~StreamingReceiver();

    virtual bool supportsPacketStreaming(cGate *gate) const override { return true; }
};

} // namespace inet

#endif // ifndef __INET_STREAMINGRECEIVER_H

