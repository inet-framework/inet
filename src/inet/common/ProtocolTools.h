//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

template<typename T>
const Ptr<T> removeProtocolHeader(Packet *packet)
{
    packet->removeTagIfPresent<PacketProtocolTag>();
    packet->trim(); // TODO breaks fingerprints, but why not? packet->trimHeaders();
    return packet->removeAtFront<T>();
}

} // namespace inet

#endif

