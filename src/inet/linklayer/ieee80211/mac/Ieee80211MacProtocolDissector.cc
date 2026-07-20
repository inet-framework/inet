//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/Ieee80211MacProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/ieee802/Ieee802EpdHeader_m.h"
#include "inet/linklayer/ieee80211/llc/LlcProtocolTag_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ieee80211Mac, Ieee80211MacProtocolDissector);

const Protocol *Ieee80211MacProtocolDissector::computeLlcProtocol(Packet *packet) const
{
    if (const auto& llcTag = packet->findTag<ieee80211::LlcProtocolTag>())
        return llcTag->getProtocol();
    else if (const auto& channelTag = packet->findTag<physicallayer::Ieee80211ChannelInd>()) {
        // EtherType protocol discrimination is mandatory for deployments in the 5.9 GHz band
        if (channelTag->getChannel()->getBand() == &physicallayer::Ieee80211CompliantBands::band5_9GHz)
            return &Protocol::ieee802epd;
    }
    const auto& header = packet->peekAtFront();
    if (dynamicPtrCast<const Ieee8022LlcHeader>(header) != nullptr)
        return &Protocol::ieee8022llc;
    else if (dynamicPtrCast<const Ieee802EpdHeader>(header) != nullptr)
        return &Protocol::ieee802epd;
    else
        return nullptr;
}

void Ieee80211MacProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<inet::ieee80211::Ieee80211MacHeader>();
    const auto& trailer = packet->popAtBack<inet::ieee80211::Ieee80211MacTrailer>(B(4));
    callback.startProtocolDataUnit(&Protocol::ieee80211Mac);
    callback.visitChunk(header, &Protocol::ieee80211Mac);
    // TODO fragmentation & aggregation
    if (auto dataHeader = dynamicPtrCast<const inet::ieee80211::Ieee80211DataHeader>(header)) {
        if (packet->getDataLength() == b(0)) {
            // the frame body is empty: a Null or QoS-Null data frame (and the
            // CF-Poll/CF-Ack data subtypes) carry no payload, so there is nothing to dissect
        }
        else if (dataHeader->getMoreFragments() || dataHeader->getFragmentNumber() != 0)
            callback.dissectPacket(packet, nullptr);
        else if (dataHeader->getAMsduPresent()) {
            auto originalTrailerPopOffset = packet->getBackOffset();
            int paddingLength = 0;
            while (packet->getDataLength() > B(0)) {
                packet->setFrontOffset(packet->getFrontOffset() + B(paddingLength == 4 ? 0 : paddingLength));
                const auto& msduSubframeHeader = packet->popAtFront<ieee80211::Ieee80211MsduSubframeHeader>();
                auto msduEndOffset = packet->getFrontOffset() + B(msduSubframeHeader->getLength());
                packet->setBackOffset(msduEndOffset);
                callback.dissectPacket(packet, computeLlcProtocol(packet));
                paddingLength = 4 - (msduSubframeHeader->getChunkLength() + B(msduSubframeHeader->getLength())).get<B>() % 4;
                packet->setBackOffset(originalTrailerPopOffset);
                packet->setFrontOffset(msduEndOffset);
            }
        }
        else
            callback.dissectPacket(packet, computeLlcProtocol(packet));
    }
    else if (dynamicPtrCast<const inet::ieee80211::Ieee80211ActionFrame>(header))
        ASSERT(packet->getDataLength() == b(0));
    else if (auto mgmtHeader = dynamicPtrCast<const inet::ieee80211::Ieee80211MgmtHeader>(header)) {
        // deserialize the management-frame body as the concrete subtype named by the
        // header, so its serializer is exercised instead of leaving the body as raw
        // bytes; unknown subtypes fall back to the generic mgmt dissector
        using namespace inet::ieee80211;
        if (packet->getDataLength() > b(0)) {
            switch (mgmtHeader->getType()) {
                case ST_BEACON: callback.visitChunk(packet->popAtFront<Ieee80211BeaconFrame>(), &Protocol::ieee80211Mgmt); break;
                case ST_PROBEREQUEST: callback.visitChunk(packet->popAtFront<Ieee80211ProbeRequestFrame>(), &Protocol::ieee80211Mgmt); break;
                case ST_PROBERESPONSE: callback.visitChunk(packet->popAtFront<Ieee80211ProbeResponseFrame>(), &Protocol::ieee80211Mgmt); break;
                case ST_ASSOCIATIONREQUEST: callback.visitChunk(packet->popAtFront<Ieee80211AssociationRequestFrame>(), &Protocol::ieee80211Mgmt); break;
                case ST_ASSOCIATIONRESPONSE: callback.visitChunk(packet->popAtFront<Ieee80211AssociationResponseFrame>(), &Protocol::ieee80211Mgmt); break;
                case ST_REASSOCIATIONREQUEST: callback.visitChunk(packet->popAtFront<Ieee80211ReassociationRequestFrame>(), &Protocol::ieee80211Mgmt); break;
                case ST_REASSOCIATIONRESPONSE: callback.visitChunk(packet->popAtFront<Ieee80211ReassociationResponseFrame>(), &Protocol::ieee80211Mgmt); break;
                case ST_AUTHENTICATION: callback.visitChunk(packet->popAtFront<Ieee80211AuthenticationFrame>(), &Protocol::ieee80211Mgmt); break;
                case ST_DEAUTHENTICATION: callback.visitChunk(packet->popAtFront<Ieee80211DeauthenticationFrame>(), &Protocol::ieee80211Mgmt); break;
                case ST_DISASSOCIATION: callback.visitChunk(packet->popAtFront<Ieee80211DisassociationFrame>(), &Protocol::ieee80211Mgmt); break;
                default: callback.dissectPacket(packet, &Protocol::ieee80211Mgmt); break;
            }
        }
    }
    // TODO else if (dynamicPtrCast<const inet::ieee80211::Ieee80211ControlFrame>(header))
    else
        ASSERT(packet->getDataLength() == b(0));
    callback.visitChunk(trailer, &Protocol::ieee80211Mac);
    callback.endProtocolDataUnit(&Protocol::ieee80211Mac);
}

} // namespace inet

