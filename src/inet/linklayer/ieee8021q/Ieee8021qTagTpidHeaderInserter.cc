//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021q/Ieee8021qTagTpidHeaderInserter.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/DropEligibleTag_m.h"
#include "inet/linklayer/common/PcpTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021qTagHeader_m.h"

namespace inet {

Define_Module(Ieee8021qTagTpidHeaderInserter);

void Ieee8021qTagTpidHeaderInserter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        vlanTagType = parseVlanTagType(par("vlanTagType"));
        tpid = vlanTagType == VLAN_TAG_TYPE_S ? 0x88A8 : 0x8100;
        const char *nextProtocolAsString = par("nextProtocol");
        if (*nextProtocolAsString != '\0')
            nextProtocol = Protocol::getProtocol(nextProtocolAsString);
        defaultVlanId = par("defaultVlanId");
        defaultPcp = par("defaultPcp");
        if (defaultPcp < -1 || defaultPcp > 7)
            throw cRuntimeError("Invalid defaultPcp parameter value: %d, must be -1 or in [0, 7]", defaultPcp);
        defaultDropEligible = par("defaultDropEligible");
        userPriorityReqMappingEnabled = par("mapUserPriorityReqToPcpReq");
        removeUserPriorityReq = par("removeUserPriorityReq");
        if (removeUserPriorityReq && !userPriorityReqMappingEnabled)
            throw cRuntimeError("The removeUserPriorityReq parameter requires mapUserPriorityReqToPcpReq");
        pcpIndMappingEnabled = par("mapPcpIndToUserPriorityInd");
        WATCH(tpid);
    }
}

void Ieee8021qTagTpidHeaderInserter::processPacket(Packet *packet)
{
    auto header = makeShared<Ieee8021qTagTpidHeader>();

    header->setTpid(tpid);

    if (userPriorityReqMappingEnabled)
        mapUserPriorityReqToPcpReq(packet);

    auto pcpReq = removePcpReqIfPresent(packet, vlanTagType);
    int pcp = defaultPcp;
    if (pcpReq != nullptr) {
        pcp = pcpReq->getPcp();
        if (pcp < 0 || pcp > 7)
            throw cRuntimeError("Invalid PCP request: %d, must be in [0, 7]", pcp);
    }
    if (pcp != -1) {
        EV_INFO << "Setting PCP" << EV_FIELD(pcp) << EV_ENDL;
        header->setPcp(pcp);
        addPcpIndIfAbsent(packet, vlanTagType)->setPcp(pcp);
    }

    auto vlanReq = removeVlanReqIfPresent(packet, vlanTagType);
    auto vlanId = vlanReq != nullptr ? vlanReq->getVlanId() : defaultVlanId;
    if (vlanId != -1) {
        EV_INFO << "Setting VID" << EV_FIELD(vid, vlanId) << EV_ENDL;
        header->setVid(vlanId);
        addVlanIndIfAbsent(packet, vlanTagType)->setVlanId(vlanId);
    }

    auto dropEligibleReq = removeDropEligibleReqIfPresent(packet, vlanTagType);
    auto dropEligible = dropEligibleReq != nullptr ? (dropEligibleReq->getDropEligible() ? 1 : 0) : defaultDropEligible;
    if (dropEligible >= 0) {
        EV_INFO << "Setting dropEligible" << EV_FIELD(dei, dropEligible) << EV_ENDL;
        header->setDei(dropEligible > 0);
        addDropEligibleIndIfAbsent(packet, vlanTagType)->setDropEligible(dropEligible);
    }

    if (pcpIndMappingEnabled)
        mapPcpIndToUserPriorityInd(packet);

    packet->insertAtFront(header);
    auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
    packetProtocolTag->setFrontOffset(packetProtocolTag->getFrontOffset() + header->getChunkLength());
    if (nextProtocol != nullptr)
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(nextProtocol);
}

void Ieee8021qTagTpidHeaderInserter::mapUserPriorityReqToPcpReq(Packet *packet)
{
    // the user priority belongs to the frame, not to this tag, which only encodes it
    // (IEEE 802.1Q-2018 6.9.3); the request to the layer below carries it unchanged
    // (6.9.2), so the tags below this one and the MAC get it too, unless it is removed
    auto userPriorityReq = packet->findTag<UserPriorityReq>();
    if (userPriorityReq == nullptr)
        return;
    if (findPcpReq(packet, vlanTagType) == nullptr) {
        int userPriority = userPriorityReq->getUserPriority();
        if (userPriority < 0 || userPriority > 7)
            throw cRuntimeError("Invalid user priority request: %d, must be in [0, 7]", userPriority);
        addPcpReqIfAbsent(packet, vlanTagType)->setPcp(userPriority);
    }
    if (removeUserPriorityReq)
        packet->removeTag<UserPriorityReq>();
}

void Ieee8021qTagTpidHeaderInserter::mapPcpIndToUserPriorityInd(Packet *packet)
{
    auto pcpInd = findPcpInd(packet, vlanTagType);
    if (pcpInd != nullptr)
        packet->addTagIfAbsent<UserPriorityInd>()->setUserPriority(pcpInd->getPcp());
}

} // namespace inet

