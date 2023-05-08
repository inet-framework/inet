//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DSCPMARKER_H
#define __INET_DSCPMARKER_H

#include "inet/common/ModuleRefByGate.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/Protocol.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {

/**
 * DSCP Marker.
 */
class INET_API DscpMarker : public queueing::PacketProcessorBase, public queueing::IPassivePacketSink, public queueing::IActivePacketSource
{
  protected:
    cGate *outputGate = nullptr;
    ModuleRefByGate<IPassivePacketSink> consumer;

    std::vector<int> dscps;

    int numRcvd = 0;
    int numMarked = 0;

    static simsignal_t packetMarkedSignal;

  public:
    DscpMarker() {}

    virtual bool supportsPacketPushing(const cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return false; }

    virtual queueing::IPassivePacketSink *getConsumer(const cGate *gate) override { return this; }

    virtual bool canPushSomePacket(const cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return true; }

    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Invalid operation"); }

    virtual void handleCanPushPacketChanged(const cGate *gate) override {}

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void refreshDisplay() const override;

    virtual bool markPacket(Packet *msg, int dscp);

  public:
    virtual void pushPacket(Packet *packet, const cGate *gate) override;

    virtual void handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful) override {}
};

} // namespace inet

#endif

