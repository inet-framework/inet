// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inet/linklayer/ieee80211/mac/Ieee80211MeshMac.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211SubtypeTag_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IRtsPolicy.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MeshFrame_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211MeshMac);

void Ieee80211MeshMac::initialize(int stage)
{
    Ieee80211Mac::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        meshManagement.reference(this, "meshManagementModule", true);
    if (stage == INITSTAGE_LAST && (!mib->qos || mib->mode != Ieee80211Mib::MESH || modeSet->isHtOperationSupported()))
        throw cRuntimeError("This mesh stage requires an OFDM QoS mesh interface");
    if (stage == INITSTAGE_LAST) {
        auto radioModule = check_and_cast<cModule *>(radio.get());
        if (strcmp(par("modeSet").stringValue(), "a") != 0
                || (radioModule->hasPar("bandName") && strcmp(radioModule->par("bandName").stringValue(), "5.9 GHz") == 0))
            throw cRuntimeError("The OFDM mesh reference profile requires 802.11a outside EPD-required 5.9 GHz channels");
        auto service = hcf->getSubmodule("originatorMacDataService");
        auto rts = check_and_cast<IRtsPolicy *>(hcf->getSubmodule("rtsPolicy"));
        if (hcf->getSubmodule("rateControl") || service->getSubmodule("fragmentationPolicy")
                || service->getSubmodule("msduAggregationPolicy") || service->getSubmodule("mpduAggregationPolicy")
                || hcf->par("isBlockAckSupported").boolValue() || rts->getRtsThreshold() <= modeSet->getMode(0)->getMpduMaxLength())
            throw cRuntimeError("The OFDM mesh profile requires fixed rates, no fragmentation/aggregation/Block Ack, and RTS disabled");
    }
}

void Ieee80211MeshMac::handleMgmtPacket(Packet *packet)
{
    auto subtype = packet->getTag<Ieee80211SubtypeReq>()->getSubtype();
    Ptr<Ieee80211MgmtHeader> header;
    if (subtype == ST_ACTION) {
        auto action = makeShared<Ieee80211MeshActionFrame>();
        auto body = packet->peekAtFront<Chunk>();
        if (dynamicPtrCast<const Ieee80211MeshPeeringConfirmFrame>(body)) {
            action->setCategory(MESH_SELF_PROTECTED_ACTION);
            action->setMeshAction(MESH_PEERING_CONFIRM);
        }
        else if (dynamicPtrCast<const Ieee80211MeshPeeringOpenFrame>(body)) {
            action->setCategory(MESH_SELF_PROTECTED_ACTION);
            action->setMeshAction(MESH_PEERING_OPEN);
        }
        else if (dynamicPtrCast<const Ieee80211MeshPeeringCloseFrame>(body)) {
            action->setCategory(MESH_SELF_PROTECTED_ACTION);
            action->setMeshAction(MESH_PEERING_CLOSE);
        }
        else {
            action->setCategory(MESH_ACTION);
            action->setMeshAction(dynamicPtrCast<const Ieee80211MeshLinkMetricFrame>(body)
                    ? MESH_LINK_METRIC_REPORT : MESH_HWMP_PATH_SELECTION);
        }
        header = action;
    }
    else {
        header = makeShared<Ieee80211MgmtHeader>();
        header->setType(static_cast<Ieee80211FrameType>(subtype));
    }
    header->setReceiverAddress(packet->getTag<MacAddressReq>()->getDestAddress());
    // IEEE Std 802.11-2024, 9.3.3.1 c1/c4v: Probe Request identifies
    // the intended recipient (or wildcard); other mesh management uses the TA.
    header->setAddress3(subtype == ST_PROBEREQUEST ? header->getReceiverAddress() : mib->address);
    packet->insertAtFront(header);
    packet->insertAtBack(makeShared<Ieee80211MacTrailer>());
    processUpperFrame(packet, header);
}

void Ieee80211MeshMac::unsupportedData(Packet *packet)
{
    PacketDropDetails details;
    details.setReason(OTHER_PACKET_DROP);
    emit(packetDroppedSignal, packet, &details);
    delete packet;
}

void Ieee80211MeshMac::handleUpperPacket(Packet *packet)
{
    unsupportedData(packet);
}

void Ieee80211MeshMac::handleLowerMessage(cMessage *message)
{
    // Supply the known interface role before LayeredProtocolBase emits the
    // received-packet observation. Address roles and QoS bit 8 alone are ambiguous.
    if (auto packet = dynamic_cast<Packet *>(message)) {
        auto first = packet->peekAtFront<Chunk>();
        auto data = dynamicPtrCast<const Ieee80211DataHeader>(first);
        // Inspect only fixed Frame Control bits for a raw input. Parsing a
        // nondata Action header here could consume its variable body and FCS.
        if (!data && packet->getDataLength() >= B(24)
                && (packet->peekAtFront<BytesChunk>(B(1))->getBytes()[0] & 0x0C) == 0x08)
            data = packet->peekAtFront<Ieee80211MeshDataHeader>(b(-1),
                    Chunk::PF_ALLOW_NULLPTR | Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE);
        if (data && data->isCorrect() && data->isComplete() && !dynamicPtrCast<const Ieee80211MeshDataHeader>(first)) {
            auto mesh = makeShared<Ieee80211MeshDataHeader>();
            mesh->Ieee80211DataHeader::operator=(*data);
            // The cached received bytes remain identical, including reserved bits,
            // so the subsequent normal FCS check still sees the on-air representation.
            packet->popAtFront(data->getChunkLength());
            packet->insertAtFront(mesh);
        }
    }
    Ieee80211Mac::handleLowerMessage(message);
}

void Ieee80211MeshMac::sendUpFrame(Packet *packet)
{
    Enter_Method("sendUpFrame");
    take(packet);
    auto header = packet->peekAtFront<Ieee80211MacHeader>();
    if (header->getFrameType() != 0) {
        unsupportedData(packet);
        return;
    }
    packet->popAtBack<Ieee80211MacTrailer>(B(4));
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211Mgmt);
    send(packet, "mgmtOut");
}

void Ieee80211MeshMac::prepareFrameTransmission(Packet *packet, simtime_t ifs,
        const physicallayer::IIeee80211Mode *mode)
{
    meshManagement->prepareManagementTransmission(packet, ifs, mode);
}

void Ieee80211MeshMac::frameTransmissionComplete(const Packet *packet)
{
    meshManagement->managementTransmissionComplete(packet);
}

bool Ieee80211MeshMac::isFrameTransmissionEligible(const Packet *packet, simtime_t plannedStart) const
{
    // IEEE Std 802.11-2024, 14.14.3.1: pending traffic waits behind the TBTT beacon.
    if (!meshManagement->isMeshManagementActive())
        return false;
    if (packet->peekAtFront<Ieee80211MacHeader>()->getType() == ST_BEACON)
        return true;
    return !meshManagement->isBeaconPending() && plannedStart < meshManagement->getNextTbtt();
}

bool Ieee80211MeshMac::isDataReceptionSupported(const Ptr<const Ieee80211DataHeader>&) const
{
    return false;
}

} // namespace ieee80211
} // namespace inet
