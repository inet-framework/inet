//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee802154/Ieee802154ProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/ieee802154/Ieee802154MacHeader_m.h"
#include "inet/linklayer/ieee802154/Ieee802154PayloadProtocolTag_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ieee802154, Ieee802154ProtocolDissector);

void Ieee802154ProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<Ieee802154MacHeader>();
    callback.startProtocolDataUnit(&Protocol::ieee802154);
    callback.visitChunk(header, &Protocol::ieee802154);
    const Protocol *payloadProtocol = nullptr;
    if (auto tag = packet->findTag<Ieee802154PayloadProtocolTag>())
        payloadProtocol = tag->getProtocol();
    callback.dissectPacket(packet, payloadProtocol);

//    auto paddingLength = packet->getDataLength();
//    if (paddingLength > b(0)) {
//        const auto& padding = packet->popHeader(paddingLength);
//        callback.visitChunk(padding, &Protocol::ieee802154);
//    }
    callback.endProtocolDataUnit(&Protocol::ieee802154);
}

} // namespace inet

