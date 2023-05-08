//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INTERPACKETGAPINSERTER_H
#define __INET_INTERPACKETGAPINSERTER_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

using namespace inet::queueing;

extern template class ClockUserModuleMixin<PacketPusherBase>;

class INET_API InterpacketGapInserter : public ClockUserModuleMixin<PacketPusherBase>
{
  protected:
    cPar *durationPar = nullptr;
    ClockEvent *timer = nullptr;
    ClockEvent *progress = nullptr;

    clocktime_t packetDelay;
    clocktime_t packetStartTime;
    clocktime_t packetEndTime;

    bps streamDatarate = bps(NaN);

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void receivePacketStart(cPacket *packet, cGate *gate, double datarate);
    virtual void receivePacketProgress(cPacket *packet, cGate *gate, double datarate, int bitPosition, simtime_t timePosition, int extraProcessableBitLength, simtime_t extraProcessableDuration);
    virtual void receivePacketEnd(cPacket *packet, cGate *gate, double datarate);

    virtual void pushOrSendOrSchedulePacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0));

    virtual std::string resolveDirective(char directive) const override;

  public:
    virtual ~InterpacketGapInserter();

    virtual IPassivePacketSink *getConsumer(const cGate *gate) override { return consumer; }

    virtual bool supportsPacketPushing(const cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return false; }

    virtual bool canPushSomePacket(const cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override;

    virtual void pushPacket(Packet *packet, const cGate *gate) override;

    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override;
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override;

    virtual void handleCanPushPacketChanged(const cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;
};

} // namespace inet

#endif

