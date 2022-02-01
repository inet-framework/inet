//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/icmpv6/Icmpv6ProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/icmpv6/Icmpv6.h"
#include "inet/networklayer/ipv6/Ipv6Header.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::icmpv6, Icmpv6ProtocolDissector);

void Icmpv6ProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    bool isBadPacket = !Icmpv6::verifyCrc(packet);
    const auto& header = packet->popAtFront<Icmpv6Header>();
    callback.startProtocolDataUnit(&Protocol::icmpv6);
    if (isBadPacket)
        callback.markIncorrect();
    callback.visitChunk(header, &Protocol::icmpv6);
    if (header->getType() < 128) {
        // ICMPv6 ERROR
        // TODO packet contains a complete Ipv6Header and the first 8 bytes of transport header (or ICMPv6). (protocol specified in Ipv6Header.)
        callback.dissectPacket(packet, nullptr);
    }
    else {
        // ICMPv6 INFO packets (e.g. ping), ICMPv6 Neighbour Discovery
        if (packet->getDataLength() > b(0))
            callback.dissectPacket(packet, nullptr);
    }
    callback.endProtocolDataUnit(&Protocol::icmpv6);
}

} // namespace inet

