//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/mrp/CfmProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/mrp/CfmContinuityCheckMessage_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ieee8021qCFM, CfmProtocolDissector);

void CfmProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    callback.startProtocolDataUnit(&Protocol::ieee8021qCFM);
    // The opcode -- the second octet of the CFM common header -- selects the message
    // type. Only the Continuity Check Message is modelled; any other message, and the
    // TLVs that may follow the fixed part of a CCM, stay raw bytes.
    const int CFM_OPCODE_CCM = 1;
    if (packet->peekDataAt<BytesChunk>(B(1), B(1))->getBytes()[0] == CFM_OPCODE_CCM)
        callback.visitChunk(packet->popAtFront<CfmContinuityCheckMessage>(), &Protocol::ieee8021qCFM);
    // the TLV list that follows: every TLV is a chunk, the one that ends the list by its
    // own class and the rest by the raw one, which holds what it could not read
    while (packet->getDataLength() > b(0)) {
        const auto& tlv = packet->popAtFront<CfmTlvBase>();
        callback.visitChunk(tlv, &Protocol::ieee8021qCFM);
        if (dynamicPtrCast<const CfmEndTlv>(tlv) != nullptr)
            break;
    }
    // whatever is left is the padding to the minimum frame size
    if (packet->getDataLength() > b(0))
        callback.dissectPacket(packet, nullptr);
    callback.endProtocolDataUnit(&Protocol::ieee8021qCFM);
}

} // namespace inet
