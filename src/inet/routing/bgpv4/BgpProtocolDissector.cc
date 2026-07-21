//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/bgpv4/BgpProtocolDissector.h"

#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/routing/bgpv4/bgpmessage/BgpHeader_m.h"

namespace inet {

using namespace bgp;

Register_Protocol_Dissector(&Protocol::bgp, BgpProtocolDissector);

void BgpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    callback.startProtocolDataUnit(&Protocol::bgp);
    // A TCP segment may carry several concatenated BGP messages. Each BGP header is a
    // 16-byte marker + 2-byte length + 1-byte type; peek the type (at offset 18) to pop
    // the concrete message type, so its serializer -- not the generic BgpHeader one -- is
    // exercised.
    while (packet->getDataLength() >= BGP_HEADER_OCTETS) {
        uint8_t type = packet->peekDataAt<BytesChunk>(B(18), B(1))->getBytes()[0];
        switch (type) {
            case BGP_OPEN: callback.visitChunk(packet->popAtFront<BgpOpenMessage>(), &Protocol::bgp); break;
            case BGP_UPDATE: callback.visitChunk(packet->popAtFront<BgpUpdateMessage>(), &Protocol::bgp); break;
            case BGP_KEEPALIVE: callback.visitChunk(packet->popAtFront<BgpKeepAliveMessage>(), &Protocol::bgp); break;
            default: callback.visitChunk(packet->popAtFront<BgpHeader>(), &Protocol::bgp); break;
        }
    }
    if (packet->getDataLength() > b(0))
        callback.dissectPacket(packet, nullptr);
    callback.endProtocolDataUnit(&Protocol::bgp);
}

} // namespace inet
