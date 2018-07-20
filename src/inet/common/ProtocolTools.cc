//
// Copyright (C) 2017 OpenSim Ltd.
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
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

