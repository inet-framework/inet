//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/common/PacketPushToSend.h"

namespace inet {
namespace queueing {

Define_Module(PacketPushToSend);

void PacketPushToSend::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    handlePacketProcessed(packet);
    send(packet, "out");
    updateDisplayString();
}

} // namespace queueing
} // namespace inet

