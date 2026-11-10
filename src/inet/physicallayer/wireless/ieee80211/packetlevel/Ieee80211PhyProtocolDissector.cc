//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211PhyProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/linklayer/ieee80211/mac/aggregation/Ieee80211Psdu.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ieee80211FhssPhy, Ieee80211PhyProtocolDissector);
Register_Protocol_Dissector(&Protocol::ieee80211IrPhy, Ieee80211PhyProtocolDissector);
Register_Protocol_Dissector(&Protocol::ieee80211DsssPhy, Ieee80211PhyProtocolDissector);
Register_Protocol_Dissector(&Protocol::ieee80211HrDsssPhy, Ieee80211PhyProtocolDissector);
Register_Protocol_Dissector(&Protocol::ieee80211OfdmPhy, Ieee80211PhyProtocolDissector);
Register_Protocol_Dissector(&Protocol::ieee80211ErpOfdmPhy, Ieee80211PhyProtocolDissector);
Register_Protocol_Dissector(&Protocol::ieee80211HtPhy, Ieee80211PhyProtocolDissector);
Register_Protocol_Dissector(&Protocol::ieee80211VhtPhy, Ieee80211PhyProtocolDissector);

void Ieee80211PhyProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto protocolTag = packet->getTag<PacketProtocolTag>()->getProtocol();
    callback.startProtocolDataUnit(protocolTag);
    auto originalBackOffset = packet->getBackOffset();
    auto payloadEndOffset = packet->getFrontOffset();
    const auto& header = physicallayer::Ieee80211Radio::popIeee80211PhyHeaderAtFront(packet, b(-1), Chunk::PF_ALLOW_INCORRECT);
    callback.visitChunk(header, protocolTag);
    payloadEndOffset += header->getChunkLength() + header->getLengthField();
    bool incorrect = (payloadEndOffset > originalBackOffset || b(header->getLengthField()) < header->getChunkLength());
    if (incorrect) {
        callback.markIncorrect();
        payloadEndOffset = originalBackOffset;
    }
    packet->setBackOffset(payloadEndOffset);
    auto request = packet->findTag<physicallayer::Ieee80211ModeReq>();
    auto indication = packet->findTag<physicallayer::Ieee80211ModeInd>();
    bool single = indication ? indication->getTxVector().singleMpdu : request && request->getTxVector().singleMpdu;
    if (single) {
        ieee80211::Ieee80211SingleMpduLayout layout;
        if (!ieee80211::findSingleMpdu(packet, b(0), B(packet->getDataLength()), layout)) {
            callback.markIncorrect();
            callback.visitChunk(packet->popAtFront(packet->getDataLength()), protocolTag);
        }
        else {
            callback.visitChunk(packet->popAtFront(layout.memberOffset), protocolTag);
            auto suffixEnd = packet->getBackOffset();
            packet->setBackOffset(packet->getFrontOffset() + layout.memberLength);
            callback.dissectPacket(packet, &Protocol::ieee80211Mac);
            packet->setBackOffset(suffixEnd);
            if (layout.suffixLength != B(0))
                callback.visitChunk(packet->popAtFront(layout.suffixLength), protocolTag);
        }
    }
    else
        callback.dissectPacket(packet, &Protocol::ieee80211Mac);
    packet->setBackOffset(originalBackOffset);
    auto paddingLength = packet->getDataLength();
    if (paddingLength > b(0)) {
        const auto& padding = packet->popAtFront(paddingLength);
        callback.visitChunk(padding, protocolTag);
    }
    callback.endProtocolDataUnit(protocolTag);
}

} // namespace inet

