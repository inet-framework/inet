//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PREEMPTINGSERVER_H
#define __INET_PREEMPTINGSERVER_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketServerBase.h"

namespace inet {

extern template class ClockUserModuleMixin<queueing::PacketServerBase>;

namespace queueing {

using namespace inet::queueing;

class INET_API PreemptingServer : public ClockUserModuleMixin<PacketServerBase>
{
  protected:
    bps datarate = bps(NaN);

    Packet *streamedPacket = nullptr;

    ClockEvent *timer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual bool isStreaming() const { return streamedPacket != nullptr; }
    virtual bool canStartStreaming() const;

    virtual void startStreaming();
    virtual void endStreaming();

  public:
    virtual ~PreemptingServer() { delete streamedPacket; cancelAndDeleteClockEvent(timer); }

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handleCanPullPacketChanged(cGate *gate) override;

    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
};

} // namespace queueing
} // namespace inet

#endif

