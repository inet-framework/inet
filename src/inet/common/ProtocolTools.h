//
// Copyright (C) 2017 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_PROTOCOLTOOLS_H
#define __INET_PROTOCOLTOOLS_H

#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"

namespace inet {

const Protocol *findPacketProtocol(Packet *packet);
const Protocol& getPacketProtocol(Packet *packet);

void insertProtocolHeader(Packet *packet, const Protocol& protocol, const Ptr<Chunk>& header);

template <typename T>
const Ptr<T> removeProtocolHeader(Packet *packet)
{
    packet->removeTagIfPresent<PacketProtocolTag>();
    packet->trim(); // TODO: breaks fingerprints, but why not? packet->trimHeaders();
    return packet->removeAtFront<T>();
}

};

#endif

