//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/icmpv6/Icmpv6ProtocolDissector.h"

#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/icmpv6/Icmpv6.h"
#include "inet/networklayer/ipv6/Ipv6Header.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::icmpv6, Icmpv6ProtocolDissector);

void Icmpv6ProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    // MLD messages -- Multicast Listener Query (130), Report (131), Done (132) and the
    // MLDv2 Report (143) -- are ICMPv6 messages with their own chunk types (MldMessage /
    // Mldv2Report, which derive from Icmpv6Header). Hand them to the MLD dissector so the
    // MLD serializers run; the generic ICMPv6 header serializer cannot represent them.
    //
    // Read the type from a field chunk directly. Reading it as a byte serializes the chunk,
    // and the serializer throws on every type it does not know: a node must accept such a
    // message (RFC 4443 section 2.4), so the dissector must not fail on one either.
    uint8_t type;
    if (auto header = dynamicPtrCast<const Icmpv6Header>(packet->peekAtFront()))
        type = header->getType();
    else
        type = packet->peekDataAt<BytesChunk>(b(0), B(1))->getBytes()[0];
    if (type == 130 || type == 131 || type == 132 || type == 143) {
        callback.dissectPacket(packet, &Protocol::mld);
        return;
    }
    bool isBadPacket = !Icmpv6::verifyChecksum(packet);
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

