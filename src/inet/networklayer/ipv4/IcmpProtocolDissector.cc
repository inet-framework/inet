//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipv4/IcmpProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/ipv4/IcmpHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::icmpv4, IcmpProtocolDissector);

void IcmpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<IcmpHeader>();
    callback.startProtocolDataUnit(&Protocol::icmpv4);
    callback.visitChunk(header, &Protocol::icmpv4);
    switch (header->getType()) {
        case ICMP_DESTINATION_UNREACHABLE:
        case ICMP_TIME_EXCEEDED:
        case ICMP_PARAMETER_PROBLEM: {
            // TODO packet contains a complete Ipv4Header and the first 8 bytes of transport header (or icmp). (protocol specified in Ipv4Header.)
            callback.dissectPacket(packet, nullptr);
            break;
        }
        default:
            callback.dissectPacket(packet, nullptr);
            break;
    }
    callback.endProtocolDataUnit(&Protocol::icmpv4);
}

} // namespace inet

