//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/xmac/XMacProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/xmac/XMacHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::xmac, XMacProtocolDissector);

void XMacProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<XMacHeaderBase>();
    callback.startProtocolDataUnit(&Protocol::xmac);
    callback.visitChunk(header, &Protocol::xmac);
    if (header->getType() == XMAC_DATA) {
        auto dataHeader = dynamicPtrCast<const XMacDataFrameHeader>(header);
        auto payloadProtocol = ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(dataHeader->getNetworkProtocol());
        callback.dissectPacket(packet, payloadProtocol);
    }
    ASSERT(packet->getDataLength() == B(0));
    callback.endProtocolDataUnit(&Protocol::xmac);
}

} // namespace inet

