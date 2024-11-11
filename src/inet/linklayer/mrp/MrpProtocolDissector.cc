//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/mrp/MrpProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/mrp/MrpPdu_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::mrp, MrpProtocolDissector);

void MrpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback &callback) const
{
    callback.startProtocolDataUnit(&Protocol::mrp);

    auto version = packet->popAtFront<MrpVersion>();
    callback.visitChunk(version, &Protocol::mrp);

    bool endReached = false;
    while (!endReached) {
        auto tlv = packet->peekAtFront<MrpTlvHeader>();
        switch (tlv->getHeaderType()) {
            case END: endReached = true; tlv = packet->popAtFront<MrpEnd>(); break;
            case COMMON: tlv = packet->popAtFront<MrpCommon>(); break;
            case TEST: tlv = packet->popAtFront<MrpTest>(); break;
            case TOPOLOGYCHANGE: tlv = packet->popAtFront<MrpTopologyChange>(); break;
            case LINKDOWN: case LINKUP: tlv = packet->popAtFront<MrpLinkChange>(); break;
            case INTEST: tlv = packet->popAtFront<MrpInTest>(); break;
            case INTOPOLOGYCHANGE: tlv = packet->popAtFront<MrpInTopologyChange>(); break;
            case INLINKDOWN: case INLINKUP: tlv = packet->popAtFront<MrpInLinkChange>(); break;
            case INLINKSTATUSPOLL: tlv = packet->popAtFront<MrpInLinkStatusPoll>(); break;
            case OPTION: tlv = packet->popAtFront<MrpOption>(); break;
            default: throw cRuntimeError("Unknown TLV type %d", tlv->getHeaderType()); break;
        }
        callback.visitChunk(tlv, &Protocol::mrp);
    }

    callback.endProtocolDataUnit(&Protocol::mrp);
}

} // namespace inet
