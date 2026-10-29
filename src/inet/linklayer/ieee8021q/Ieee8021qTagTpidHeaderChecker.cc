//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021q/Ieee8021qTagTpidHeaderChecker.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/DropEligibleTag_m.h"
#include "inet/linklayer/common/PcpTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021qTagHeader_m.h"

namespace inet {

Define_Module(Ieee8021qTagTpidHeaderChecker);

void Ieee8021qTagTpidHeaderChecker::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        vlanTagType = parseVlanTagType(par("vlanTagType"));
        tpid = vlanTagType == VLAN_TAG_TYPE_S ? 0x88A8 : 0x8100;
        vlanIdFilter = check_and_cast<cValueArray *>(par("vlanIdFilter").objectValue());
        WATCH(tpid);
    }
}

void Ieee8021qTagTpidHeaderChecker::processPacket(Packet *packet)
{
    const auto& header = packet->popAtFront<Ieee8021qTagTpidHeader>();
    // IEEE 802.1Q-2018 6.9.1 h): the priority of a tagged frame is decoded from the
    // tag header, and replaces the one that the layer below indicated, so in a stack
    // the innermost tag, which is checked last, determines the user priority
    packet->addTagIfAbsent<UserPriorityInd>()->setUserPriority(header->getPcp());
    addPcpIndIfAbsent(packet, vlanTagType)->setPcp(header->getPcp());
    addVlanIndIfAbsent(packet, vlanTagType)->setVlanId(header->getVid());
    addDropEligibleIndIfAbsent(packet, vlanTagType)->setDropEligible(header->getDei());
    auto packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
    packetProtocolTag->setFrontOffset(packetProtocolTag->getFrontOffset() - header->getChunkLength());
}

void Ieee8021qTagTpidHeaderChecker::dropPacket(Packet *packet)
{
    EV_WARN << "Received packet " << packet->getName() << " is not accepted, dropping packet.\n";
    PacketFilterBase::dropPacket(packet, OTHER_PACKET_DROP);
}

bool Ieee8021qTagTpidHeaderChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = packet->peekAtFront<Ieee8021qTagTpidHeader>();
    if (header->getTpid() != tpid)
        return false;
    else {
        auto vlanId = header->getVid();
        if (vlanIdFilter->size() == 0)
            return true;
        else {
            for (int i = 0; i < vlanIdFilter->size(); i++)
                if (vlanIdFilter->get(i).intValue() == vlanId)
                    return true;
            return false;
        }
    }
}

} // namespace inet

