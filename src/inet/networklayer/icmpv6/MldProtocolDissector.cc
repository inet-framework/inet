//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/icmpv6/MldProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/icmpv6/MldMessage_m.h"
#include "inet/networklayer/icmpv6/Mldv2Message_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::mld, MldProtocolDissector);

void MldProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    callback.startProtocolDataUnit(&Protocol::mld);
    // The MLDv2 Report (ICMPv6 type 143) is not an MldMessage; peek the ICMPv6 type to
    // decide which concrete chunk to pop. MldMessage and Mldv2Report both derive from
    // Icmpv6Header.
    const auto& icmpHeader = packet->peekAtFront<Icmpv6Header>();
    if (icmpHeader->getType() == ICMPv6_MLDv2_REPORT)
        callback.visitChunk(packet->popAtFront<Mldv2Report>(), &Protocol::mld);
    else
        callback.visitChunk(packet->popAtFront<MldMessage>(), &Protocol::mld);
    callback.endProtocolDataUnit(&Protocol::mld);
}

} // namespace inet
