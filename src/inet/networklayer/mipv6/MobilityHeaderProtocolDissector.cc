//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/mipv6/MobilityHeaderProtocolDissector.h"

#include "inet/common/Protocol.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/mipv6/MobilityHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::mobileipv6, MobilityHeaderProtocolDissector);

void MobilityHeaderProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    // The Ipv6 dissector routes IP protocol 135 here (IP_PROT_IPv6EXT_MOB ->
    // Protocol::mobileipv6). The MH Type field (3rd byte of the Mobility Header)
    // selects the concrete message, so pop that concrete subtype rather than the
    // MobilityHeader base: the coverage recorder keys on the requested pop type, and
    // the serializer's deserialize returns the matching subtype in either case.
    uint8_t mhType = packet->peekDataAt<BytesChunk>(B(2), B(1))->getBytes()[0];
    callback.startProtocolDataUnit(&Protocol::mobileipv6);
    switch (mhType) {
        case BINDING_REFRESH_REQUEST: callback.visitChunk(packet->popAtFront<BindingRefreshRequest>(), &Protocol::mobileipv6); break;
        case HOME_TEST_INIT: callback.visitChunk(packet->popAtFront<HomeTestInit>(), &Protocol::mobileipv6); break;
        case CARE_OF_TEST_INIT: callback.visitChunk(packet->popAtFront<CareOfTestInit>(), &Protocol::mobileipv6); break;
        case HOME_TEST: callback.visitChunk(packet->popAtFront<HomeTest>(), &Protocol::mobileipv6); break;
        case CARE_OF_TEST: callback.visitChunk(packet->popAtFront<CareOfTest>(), &Protocol::mobileipv6); break;
        case BINDING_UPDATE: callback.visitChunk(packet->popAtFront<BindingUpdate>(), &Protocol::mobileipv6); break;
        case BINDING_ACKNOWLEDGEMENT: callback.visitChunk(packet->popAtFront<BindingAcknowledgement>(), &Protocol::mobileipv6); break;
        case BINDING_ERROR: callback.visitChunk(packet->popAtFront<BindingError>(), &Protocol::mobileipv6); break;
        default: callback.visitChunk(packet->popAtFront<MobilityHeader>(), &Protocol::mobileipv6); break;
    }
    // the Mobility Header is a terminal (upper-layer) header; nothing should follow
    if (packet->getDataLength() > b(0))
        callback.dissectPacket(packet, nullptr);
    callback.endProtocolDataUnit(&Protocol::mobileipv6);
}

} // namespace inet
