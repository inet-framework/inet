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

#ifndef __INET_INTERPACKETGAP_H
#define __INET_INTERPACKETGAP_H

#include "inet/common/base/ClockUsingModuleMixin.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API InterPacketGap : public ClockUsingModuleMixin<PacketPusherBase>
{
  protected:
    cPar *durationPar = nullptr;
    cProgress *progress = nullptr;

    simclocktime_t packetDelay;
    simclocktime_t packetStartTime;
    simclocktime_t packetEndTime;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void receivePacketStart(cPacket *packet, cGate *gate, double datarate) override;
    virtual void receivePacketProgress(cPacket *packet, cGate *gate, double datarate, int bitPosition, simtime_t timePosition, int extraProcessableBitLength, simtime_t extraProcessableDuration) override;
    virtual void receivePacketEnd(cPacket *packet, cGate *gate, double datarate) override;

    virtual void pushOrSendOrScheduleProgress(Packet *packet, cGate *gate, int progressKind, bps datarate, b position, b extraProcessableLength = b(0));

  public:
    virtual ~InterPacketGap() { cancelAndDelete(progress); }

    virtual IPassivePacketSink *getConsumer(cGate *gate) override { return consumer; }

    virtual bool supportsPacketPushing(cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }

    virtual bool canPushSomePacket(cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override;

    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, cGate *gate, bps datarate) override;
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override;

    virtual void handleCanPushPacket(cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;

    virtual b getPushPacketProcessedLength(Packet *packet, cGate *gate) override;
};

} // namespace inet

#endif // ifndef __INET_INTERPACKETGAP_H

