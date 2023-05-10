//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SENDTOMACADDRESS_H
#define __INET_SENDTOMACADDRESS_H

#include "inet/linklayer/common/MacAddress.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API SendToMacAddress : public PacketFlowBase
{
  protected:
    MacAddress address;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;

  public:
    virtual bool canPushSomePacket(const cGate *gate) const override { return consumer.canPushSomePacket(); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return consumer.canPushPacket(packet); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;

    virtual IPassivePacketSink *getConsumer(const cGate *gate) override { return nullptr; }
    virtual void handleCanPushPacketChanged(const cGate *gate) override;

    virtual void handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;
};

} // namespace inet

#endif

