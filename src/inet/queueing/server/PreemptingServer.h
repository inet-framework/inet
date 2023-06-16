//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PREEMPTINGSERVER_H
#define __INET_PREEMPTINGSERVER_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketServerBase.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {

extern template class ClockUserModuleMixin<queueing::PacketServerBase>;

namespace queueing {

using namespace inet::queueing;

class INET_API PreemptingServer : public ClockUserModuleMixin<PacketServerBase>, public virtual IPassivePacketSink
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

    virtual void handleCanPushPacketChanged(const cGate *gate) override;
    virtual void handleCanPullPacketChanged(const cGate *gate) override;

    virtual void handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;

    virtual bool canPushSomePacket(const cGate *gate) const override { throw cRuntimeError("TODO"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { throw cRuntimeError("TODO"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override;
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }
};

} // namespace queueing
} // namespace inet

#endif

