//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/lmac/LMacProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/lmac/LMacHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::lmac, LMacProtocolDissector);

void LMacProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<LMacHeaderBase>();
    callback.startProtocolDataUnit(&Protocol::lmac);
    callback.visitChunk(header, &Protocol::lmac);
    if (header->getType() == LMAC_DATA) {
        const auto& dataHeader = dynamicPtrCast<const LMacDataFrameHeader>(header);
        auto payloadProtocol = ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(dataHeader->getNetworkProtocol());
        callback.dissectPacket(packet, payloadProtocol);
    }
    ASSERT(packet->getDataLength() == B(0));
    callback.endProtocolDataUnit(&Protocol::lmac);
}

} // namespace inet

