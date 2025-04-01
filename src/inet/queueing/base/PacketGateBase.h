//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETGATEBASE_H
#define __INET_PACKETGATEBASE_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/Units.h"
#include "inet/queueing/base/PacketFlowBase.h"
#include "inet/queueing/contract/IPacketGate.h"

namespace inet {
namespace queueing {

using namespace units::values;

class INET_API PacketGateBase : public PacketFlowBase, public TransparentProtocolRegistrationListener, public virtual IPacketGate
{
  public:
    static simsignal_t gateStateChangedSignal;

  protected:
    bps bitrate = bps(NaN);
    b extraLength = b(-1);
    simtime_t extraDuration;

    bool isOpen_ = false;

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

    virtual void processPacket(Packet *packet) override;

    virtual bool canPacketFlowThrough(Packet *packet) const;

    virtual void refreshDisplay() const override;

  public:
    virtual bool isOpen() const override { return isOpen_; }
    virtual bool isClosed() const { return !isOpen_; }
    virtual void open() override;
    virtual void close() override;

    virtual int getNumPackets() const override;
    virtual b getTotalLength() const override;
    virtual Packet* getPacket(int index) const override;
    virtual bool isEmpty() const override;
    virtual void removePacket(Packet *packet) override;
    virtual void removeAllPackets() override;

    virtual IPassivePacketSink *getConsumer(const cGate *gate) override { return this; }
    virtual IPassivePacketSource *getProvider(const cGate *gate) override { return this; }

    virtual bool supportsPacketPushing(const cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return true; }

    virtual bool canPushSomePacket(const cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override;

    virtual bool canPullSomePacket(const cGate *gate) const override;
    virtual Packet *canPullPacket(const cGate *gate) const override;

    virtual void handleCanPushPacketChanged(const cGate *gate) override;
    virtual void handleCanPullPacketChanged(const cGate *gate) override;
};

} // namespace queueing
} // namespace inet

#endif

