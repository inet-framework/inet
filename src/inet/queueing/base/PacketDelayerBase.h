//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETDELAYERBASE_H
#define __INET_PACKETDELAYERBASE_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

extern template class ClockUserModuleMixin<queueing::PacketPusherBase>;

namespace queueing {

class INET_API PacketDelayerBase : public ClockUserModuleMixin<PacketPusherBase>
{
  protected:
    int schedulingPriority = -1;
    bool scheduleZeroDelay = false;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void processPacket(Packet *packet, simtime_t sendingTime);


    virtual clocktime_t computeDelay(Packet *packet) const = 0;

  public:
    virtual void pushPacket(Packet *packet, const cGate *gate) override;

    virtual void handleCanPushPacketChanged(const cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;
};

} // namespace queueing
} // namespace inet

#endif

