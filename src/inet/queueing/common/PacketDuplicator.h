//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETDUPLICATOR_H
#define __INET_PACKETDUPLICATOR_H

#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {
namespace queueing {

class INET_API PacketDuplicator : public PacketPusherBase
{
  public:
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
};

} // namespace queueing
} // namespace inet

#endif

