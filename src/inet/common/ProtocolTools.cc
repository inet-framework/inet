//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/ProtocolTools.h"

namespace inet {

const Protocol *findPacketProtocol(Packet *packet)
{
    const auto& packetProtocolTag = packet->getTag<PacketProtocolTag>();
    return packetProtocolTag == nullptr ? nullptr : packetProtocolTag->getProtocol();
}

const Protocol& getPacketProtocol(Packet *packet)
{
    auto protocol = findPacketProtocol(packet);
    if (protocol == nullptr)
        throw cRuntimeError("Packet protocol not found");
    else
        return *protocol;
}

void insertProtocolHeader(Packet *packet, const Protocol& protocol, const Ptr<Chunk>& header)
{
    auto packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
    packetProtocolTag->setProtocol(&protocol);
    packet->insertAtFront(header);
}

} // namespace inet

