//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/arp/ipv4/ArpProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/arp/ipv4/ArpPacket_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::arp, ArpProtocolDissector);

void ArpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& arpPacket = packet->popAtFront<ArpPacket>();
    callback.startProtocolDataUnit(&Protocol::arp);
    callback.visitChunk(arpPacket, &Protocol::arp);
    callback.endProtocolDataUnit(&Protocol::arp);
}

} // namespace inet

