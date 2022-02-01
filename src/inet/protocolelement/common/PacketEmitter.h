//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETEMITTER_H
#define __INET_PACKETEMITTER_H

#include "inet/common/DirectionTag_m.h"
#include "inet/common/Protocol.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API PacketEmitter : public PacketFlowBase
{
  protected:
    simsignal_t signal;
    PacketFilter packetFilter;
    Direction direction = DIRECTION_UNDEFINED;

    Packet *processedPacket = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
    virtual void emitPacket(Packet *packet);

  public:
    virtual ~PacketEmitter() { delete processedPacket; }

    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
};

} // namespace inet

#endif

