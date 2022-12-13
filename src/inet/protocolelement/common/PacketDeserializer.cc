//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/common/PacketDeserializer.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"

namespace inet {

Define_Module(PacketDeserializer);

void PacketDeserializer::processPacket(Packet *packet)
{
    const auto& packetProtocolTag = packet->findTag<PacketProtocolTag>();
    auto protocol = packetProtocolTag != nullptr ? packetProtocolTag->getProtocol() : nullptr;
    PacketDissector::ChunkBuilder chunkBuilder;
    PacketDissector packetDissector(ProtocolDissectorRegistry::getInstance(), chunkBuilder);
    packetDissector.dissectPacket(packet, protocol);
    packet->eraseAll();
    packet->insertAtFront(chunkBuilder.getContent());
}

} // namespace inet

