//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/protocol/common/PacketDeserializer.h"

namespace inet {

Define_Module(PacketDeserializer);

void PacketDeserializer::processPacket(Packet *packet)
{
    const auto& packetProtocolTag = packet->findTag<PacketProtocolTag>();
    auto protocol = packetProtocolTag != nullptr ? packetProtocolTag->getProtocol() : nullptr;
    PacketDissector::ChunkBuilder chunkBuilder;
    PacketDissector packetDissector(ProtocolDissectorRegistry::globalRegistry, chunkBuilder);
    packetDissector.dissectPacket(packet, protocol);
    packet->eraseAll();
    packet->insertAtFront(chunkBuilder.getContent());
}

} // namespace inet

