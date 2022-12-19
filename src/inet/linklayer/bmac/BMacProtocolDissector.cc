//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/bmac/BMacProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/bmac/BMacHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::bmac, BMacProtocolDissector);

void BMacProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<BMacHeaderBase>();
    callback.startProtocolDataUnit(&Protocol::bmac);
    callback.visitChunk(header, &Protocol::bmac);
    if (header->getType() == BMAC_DATA) {
        const auto& dataHeader = dynamicPtrCast<const BMacDataFrameHeader>(header);
        auto payloadProtocol = ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(dataHeader->getNetworkProtocol());
        callback.dissectPacket(packet, payloadProtocol);
    }
    ASSERT(packet->getDataLength() == B(0));
    callback.endProtocolDataUnit(&Protocol::bmac);
}

} // namespace inet

