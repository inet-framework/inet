//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/common/PacketPushToSend.h"

namespace inet {
namespace queueing {

Define_Module(PacketPushToSend);

void PacketPushToSend::handlePushPacket(Packet *packet, cGate *gate)
{
    handlePacketProcessed(packet);
    send(packet, "out");
    updateDisplayString();
}

} // namespace queueing
} // namespace inet

