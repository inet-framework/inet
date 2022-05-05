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
        const char *vlanTagType = par("vlanTagType");
        if (!strcmp("s", vlanTagType))
            tpid = 0x88A8;
        else if (!strcmp("c", vlanTagType))
            tpid = 0x8100;
        else
            throw cRuntimeError("Unknown tag type");
        const char *nextProtocolAsString = par("nextProtocol");
        if (*nextProtocolAsString != '\0')
            nextProtocol = Protocol::getProtocol(nextProtocolAsString);
        defaultVlanId = par("defaultVlanId");
        defaultPcp = par("defaultPcp");
        defaultUserPriority = par("defaultUserPriority");
        defaultDropEligible = par("defaultDropEligible");
    }
}

void Ieee8021qTagTpidHeaderInserter::processPacket(Packet *packet)
{
    auto header = makeShared<Ieee8021qTagTpidHeader>();

    header->setTpid(tpid);

    auto userPriorityReq = packet->removeTagIfPresent<UserPriorityReq>();
    auto userPriority = userPriorityReq != nullptr ? userPriorityReq->getUserPriority() : defaultUserPriority;
    if (userPriority != -1) {
        EV_INFO << "Setting PCP" << EV_FIELD(pcp, userPriority) << EV_ENDL;
        header->setPcp(userPriority);
        packet->addTagIfAbsent<UserPriorityInd>()->setUserPriority(userPriority);
    }

    auto pcpReq = packet->removeTagIfPresent<PcpReq>();
    auto pcp = pcpReq != nullptr ? pcpReq->getPcp() : defaultPcp;
    if (pcp != -1) {
        EV_INFO << "Setting PCP" << EV_FIELD(vid, pcp) << EV_ENDL;
        header->setPcp(pcp);
        packet->addTagIfAbsent<PcpInd>()->setPcp(pcp);
    }

    auto vlanReq = packet->removeTagIfPresent<VlanReq>();
    auto vlanId = vlanReq != nullptr ? vlanReq->getVlanId() : defaultVlanId;
    if (vlanId != -1) {
        EV_INFO << "Setting VID" << EV_FIELD(vid, vlanId) << EV_ENDL;
        header->setVid(vlanId);
        packet->addTagIfAbsent<VlanInd>()->setVlanId(vlanId);
    }

    auto dropEligibleReq = packet->removeTagIfPresent<DropEligibleReq>();
    auto dropEligible = dropEligibleReq != nullptr ? (dropEligibleReq->getDropEligible() ? 1 : 0) : defaultDropEligible;
    if (dropEligible >= 0) {
        EV_INFO << "Setting dropEligible" << EV_FIELD(dei, dropEligible) << EV_ENDL;
        header->setDei(dropEligible > 0);
        packet->addTagIfAbsent<DropEligibleInd>()->setDropEligible(dropEligible);
    }

    packet->insertAtFront(header);
    auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
    packetProtocolTag->setFrontOffset(packetProtocolTag->getFrontOffset() + header->getChunkLength());
    if (nextProtocol != nullptr)
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(nextProtocol);
}

} // namespace inet

