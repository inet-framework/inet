//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/common/PacketSerializer.h"

namespace inet {

Define_Module(PacketSerializer);

void PacketSerializer::processPacket(Packet *packet)
{
    const auto& content = packet->peekAllAsBytes();
    packet->eraseAll();
    packet->insertAtFront(content);
}

} // namespace inet

