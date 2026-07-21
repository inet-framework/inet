//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/icmpv6/MldProtocolDissector.h"

#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/icmpv6/MldMessage_m.h"
#include "inet/networklayer/icmpv6/Mldv2Message_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::mld, MldProtocolDissector);

void MldProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    callback.startProtocolDataUnit(&Protocol::mld);
    // The ICMPv6 type (first byte) selects the concrete MLD chunk; pop that concrete type
    // so its serializer is exercised. MldQuery/Report/Done and the MLDv2 Query/Report all
    // derive from Icmpv6Header. A type-130 Query is MLDv1 unless it is longer than the
    // fixed 24-byte MLDv1 Query, in which case it is an MLDv2 Query.
    uint8_t type = packet->peekDataAt<BytesChunk>(b(0), B(1))->getBytes()[0];
    switch (type) {
        case ICMPv6_MLD_QUERY:
            if (packet->getDataLength() > B(24))
                callback.visitChunk(packet->popAtFront<Mldv2Query>(), &Protocol::mld);
            else
                callback.visitChunk(packet->popAtFront<MldQuery>(), &Protocol::mld);
            break;
        case ICMPv6_MLD_REPORT: callback.visitChunk(packet->popAtFront<MldReport>(), &Protocol::mld); break;
        case ICMPv6_MLD_DONE: callback.visitChunk(packet->popAtFront<MldDone>(), &Protocol::mld); break;
        case ICMPv6_MLDv2_REPORT: callback.visitChunk(packet->popAtFront<Mldv2Report>(), &Protocol::mld); break;
        default: callback.visitChunk(packet->popAtFront<MldMessage>(), &Protocol::mld); break;
    }
    callback.endProtocolDataUnit(&Protocol::mld);
}

} // namespace inet
