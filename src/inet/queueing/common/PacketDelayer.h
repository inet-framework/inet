//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETDELAYER_H
#define __INET_PACKETDELAYER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {
namespace queueing {

class INET_API PacketDelayer : public ClockUserModuleMixin<PacketPusherBase>, public TransparentProtocolRegistrationListener
{
  protected:
    virtual void handleMessage(cMessage *message) override;

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

  public:
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
};

} // namespace queueing
} // namespace inet

#endif

