//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/csmaca/CsmaCaMacProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/csmaca/CsmaCaMacHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::csmaCaMac, CsmaCaMacProtocolDissector);

void CsmaCaMacProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<CsmaCaMacHeader>();
    auto trailer = packet->popAtBack<CsmaCaMacTrailer>(B(4));
    callback.startProtocolDataUnit(&Protocol::csmaCaMac);
    callback.visitChunk(header, &Protocol::csmaCaMac);
    if (auto dataHeader = dynamicPtrCast<const CsmaCaMacDataHeader>(header)) {
        auto payloadProtocol = ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(dataHeader->getNetworkProtocol());
        callback.dissectPacket(packet, payloadProtocol);
    }
    ASSERT(packet->getDataLength() == B(0));
    callback.visitChunk(trailer, &Protocol::csmaCaMac);
    callback.endProtocolDataUnit(&Protocol::csmaCaMac);
}

} // namespace inet

