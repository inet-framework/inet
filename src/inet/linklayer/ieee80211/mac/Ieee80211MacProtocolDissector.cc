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
#include "inet/linklayer/ieee80211/mac/aggregation/Ieee80211MeshAmsdu.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MeshFrame_m.h"
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
    // Pop the FCS trailer before the header so that a header whose length is only
    // bounded by the end of the frame -- an Ieee80211ActionFrameOther, which stores the
    // unmodelled action body up to the FCS -- does not read into the FCS.
    const auto& trailer = packet->popAtBack<inet::ieee80211::Ieee80211MacTrailer>(B(4));
    const auto& header = packet->popAtFront<inet::ieee80211::Ieee80211MacHeader>();
    callback.startProtocolDataUnit(&Protocol::ieee80211Mac);
    callback.visitChunk(header, &Protocol::ieee80211Mac);
    // TODO fragmentation & aggregation
    if (header->getProtectedFrame() && packet->getDataLength() > b(0)) {
        // the frame body is encrypted (a CCMP/TKIP header followed by ciphertext), so
        // there is nothing to descend into without the key
        callback.dissectPacket(packet, nullptr);
    }
    else if (auto dataHeader = dynamicPtrCast<const inet::ieee80211::Ieee80211DataHeader>(header)) {
        if (packet->getDataLength() == b(0)) {
            // the frame body is empty: a Null or QoS-Null data frame (and the
            // CF-Poll/CF-Ack data subtypes) carry no payload, so there is nothing to dissect
            if (auto mesh = dynamicPtrCast<const ieee80211::Ieee80211MeshDataHeader>(dataHeader))
                if (mesh->getMeshControlPresent())
                    callback.markIncorrect();
        }
        else if (dataHeader->getMoreFragments() || dataHeader->getFragmentNumber() != 0)
            callback.dissectPacket(packet, nullptr);
        else if (dataHeader->getAMsduPresent()) {
            if (auto mesh = dynamicPtrCast<const ieee80211::Ieee80211MeshDataHeader>(dataHeader)) {
                std::unique_ptr<Packet> complete(packet->dup());
                complete->setFrontOffset(packet->getFrontOffset() - header->getChunkLength());
                complete->setBackOffset(packet->getBackOffset() + B(4));
                std::vector<ieee80211::Ieee80211MeshAmsduPart> parts;
                if (!ieee80211::parseMeshAmsdu(complete.get(), mesh, parts)) {
                    callback.markIncorrect();
                    callback.dissectPacket(packet, nullptr);
                }
                else {
                    auto bodyEnd = packet->getBackOffset();
                    for (const auto& part : parts) {
                        if (packet->getFrontOffset() < complete->getFrontOffset() + part.controlOffset - B(14))
                            callback.visitChunk(packet->popAtFront(complete->getFrontOffset() + part.controlOffset - B(14) - packet->getFrontOffset()), &Protocol::ieee80211Mac);
                        callback.visitChunk(packet->popAtFront<ieee80211::Ieee80211MeshMsduSubframeHeader>(B(14)), &Protocol::ieee80211Mac);
                        callback.visitChunk(packet->popAtFront<ieee80211::Ieee80211MeshControl>(B(6)), &Protocol::ieee80211Mac);
                        auto end = packet->getFrontOffset() + part.payloadLength;
                        packet->setBackOffset(end);
                        if (part.payloadLength != B(0))
                            callback.dissectPacket(packet, part.payloadLength >= B(3) ? computeLlcProtocol(packet) : nullptr);
                        packet->setBackOffset(bodyEnd);
                        packet->setFrontOffset(end);
                    }
                }
                callback.visitChunk(trailer, &Protocol::ieee80211Mac);
                callback.endProtocolDataUnit(&Protocol::ieee80211Mac);
                return;
            }
            if (dataHeader->getToDS() && dataHeader->getFromDS()) {
                // Raw four-address A-MSDU has no interface-role context here.
                // Preserve its whole MAC interval without guessing Basic versus Mesh.
                callback.dissectPacket(packet, nullptr);
                callback.visitChunk(trailer, &Protocol::ieee80211Mac);
                callback.endProtocolDataUnit(&Protocol::ieee80211Mac);
                return;
            }
            auto originalTrailerPopOffset = packet->getBackOffset();
            int paddingLength = 0;
            while (packet->getDataLength() > B(0)) {
                // every A-MSDU subframe except the last is padded to a multiple of 4
                // octets; hand the padding to the callback instead of just skipping it,
                // so the dissected content still covers the whole frame
                if (paddingLength != 0 && paddingLength != 4)
                    callback.visitChunk(packet->popAtFront(B(paddingLength)), &Protocol::ieee80211Mac);
                const auto& msduSubframeHeader = packet->popAtFront<ieee80211::Ieee80211MsduSubframeHeader>();
                callback.visitChunk(msduSubframeHeader, &Protocol::ieee80211Mac);
                auto msduEndOffset = packet->getFrontOffset() + B(msduSubframeHeader->getLength());
                packet->setBackOffset(msduEndOffset);
                callback.dissectPacket(packet, computeLlcProtocol(packet));
                paddingLength = 4 - (msduSubframeHeader->getChunkLength() + B(msduSubframeHeader->getLength())).get<B>() % 4;
                packet->setBackOffset(originalTrailerPopOffset);
                packet->setFrontOffset(msduEndOffset);
            }
        }
        else {
            bool invalidMeshControl = false;
            if (auto mesh = dynamicPtrCast<const ieee80211::Ieee80211MeshDataHeader>(dataHeader)) {
                if (mesh->getMeshControlPresent()) {
                    auto control = packet->popAtFront<ieee80211::Ieee80211MeshControl>(b(-1),
                            Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE);
                    callback.visitChunk(control, &Protocol::ieee80211Mac);
                    invalidMeshControl = control->isIncorrect() || control->isIncomplete();
                    if (invalidMeshControl)
                        callback.markIncorrect();
                }
            }
            if (packet->getDataLength() > b(0))
                callback.dissectPacket(packet, invalidMeshControl ? nullptr : computeLlcProtocol(packet));
        }
    }
    else if (auto mesh = dynamicPtrCast<const ieee80211::Ieee80211MeshActionFrame>(header)) {
        using namespace inet::ieee80211;
        auto length = packet->getDataLength();
        int flags = Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE;
        if (length == b(0))
            callback.markIncorrect();
        else if (mesh->getCategory() == MESH_ACTION && mesh->getMeshAction() == MESH_HWMP_PATH_SELECTION)
            callback.visitChunk(packet->popAtFront<Ieee80211HwmpFrame>(length, flags), &Protocol::ieee80211Mac);
        else if (mesh->getCategory() == MESH_ACTION && mesh->getMeshAction() == MESH_LINK_METRIC_REPORT)
            callback.visitChunk(packet->popAtFront<Ieee80211MeshLinkMetricFrame>(length, flags), &Protocol::ieee80211Mac);
        else if (mesh->getMeshAction() == MESH_PEERING_OPEN)
            callback.visitChunk(packet->popAtFront<Ieee80211MeshPeeringOpenFrame>(length, flags), &Protocol::ieee80211Mac);
        else if (mesh->getMeshAction() == MESH_PEERING_CONFIRM)
            callback.visitChunk(packet->popAtFront<Ieee80211MeshPeeringConfirmFrame>(length, flags), &Protocol::ieee80211Mac);
        else if (mesh->getMeshAction() == MESH_PEERING_CLOSE)
            callback.visitChunk(packet->popAtFront<Ieee80211MeshPeeringCloseFrame>(length, flags), &Protocol::ieee80211Mac);
    }
    else if (dynamicPtrCast<const inet::ieee80211::Ieee80211ActionFrame>(header)) {
        // A fully modelled action (ADDBA/DELBA) and the Ieee80211ActionFrameOther that
        // carries an unmodelled action body both consume the whole frame, so nothing
        // should remain; keep the raw fallback only as a safety net.
        if (packet->getDataLength() > b(0))
            callback.dissectPacket(packet, nullptr);
    }
    else if (auto mgmtHeader = dynamicPtrCast<const inet::ieee80211::Ieee80211MgmtHeader>(header)) {
        // deserialize the management-frame body as the concrete subtype named by the
        // header, so its serializer is exercised instead of leaving the body as raw
        // bytes; unknown subtypes fall back to the generic mgmt dissector
        using namespace inet::ieee80211;
        if (packet->getDataLength() > b(0)) {
            Ptr<const Chunk> body;
            const auto length = packet->getDataLength();
            const int flags = Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE;
            switch (mgmtHeader->getType()) {
                case ST_BEACON: body = packet->popAtFront<Ieee80211BeaconFrame>(length, flags); break;
                case ST_PROBEREQUEST: body = packet->popAtFront<Ieee80211ProbeRequestFrame>(length, flags); break;
                case ST_PROBERESPONSE: body = packet->popAtFront<Ieee80211ProbeResponseFrame>(length, flags); break;
                case ST_ASSOCIATIONREQUEST: body = packet->popAtFront<Ieee80211AssociationRequestFrame>(length, flags); break;
                case ST_ASSOCIATIONRESPONSE: body = packet->popAtFront<Ieee80211AssociationResponseFrame>(length, flags); break;
                case ST_REASSOCIATIONREQUEST: body = packet->popAtFront<Ieee80211ReassociationRequestFrame>(length, flags); break;
                case ST_REASSOCIATIONRESPONSE: body = packet->popAtFront<Ieee80211ReassociationResponseFrame>(length, flags); break;
                case ST_AUTHENTICATION: body = packet->popAtFront<Ieee80211AuthenticationFrame>(length, flags); break;
                case ST_DEAUTHENTICATION: body = packet->popAtFront<Ieee80211DeauthenticationFrame>(length, flags); break;
                case ST_DISASSOCIATION: body = packet->popAtFront<Ieee80211DisassociationFrame>(length, flags); break;
                default: callback.dissectPacket(packet, &Protocol::ieee80211Mgmt); break;
            }
            if (body) {
                callback.visitChunk(body, &Protocol::ieee80211Mgmt);
                if (body->isIncorrect() || body->isIncomplete())
                    callback.markIncorrect();
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

