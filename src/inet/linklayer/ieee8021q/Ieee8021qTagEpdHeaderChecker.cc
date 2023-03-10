//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021q/Ieee8021qTagEpdHeaderChecker.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/ProtocolUtils.h"
#include "inet/linklayer/common/DropEligibleTag_m.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/PcpTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021qTagHeader_m.h"

namespace inet {

Define_Module(Ieee8021qTagEpdHeaderChecker);

void Ieee8021qTagEpdHeaderChecker::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *vlanTagType = par("vlanTagType");
        if (!strcmp("s", vlanTagType))
            qtagProtocol = &Protocol::ieee8021qSTag;
        else if (!strcmp("c", vlanTagType))
            qtagProtocol = &Protocol::ieee8021qCTag;
        else
            throw cRuntimeError("Unknown tag type");
        vlanIdFilter = check_and_cast<cValueArray *>(par("vlanIdFilter").objectValue());
    }
    else if (stage == INITSTAGE_LINK_LAYER)
        registerProtocol(*qtagProtocol, nullptr, inputGate);
}

void Ieee8021qTagEpdHeaderChecker::handleParameterChange(const char *name)
{
    if (!strcmp(name, "vlanIdFilter"))
        vlanIdFilter = check_and_cast<cValueArray *>(par("vlanIdFilter").objectValue());
}

void Ieee8021qTagEpdHeaderChecker::processPacket(Packet *packet)
{
    const auto& header = packet->popAtFront<Ieee8021qTagEpdHeader>();
    appendEncapsulationProtocolInd(packet, qtagProtocol);
    packet->addTagIfAbsent<UserPriorityInd>()->setUserPriority(header->getPcp());
    packet->addTagIfAbsent<PcpInd>()->setPcp(header->getPcp());
    packet->addTagIfAbsent<VlanInd>()->setVlanId(header->getVid());
    packet->addTagIfAbsent<DropEligibleInd>()->setDropEligible(header->getDei());
    auto typeOrLength = header->getTypeOrLength();
    const Protocol *protocol;
    if (isIeee8023Length(typeOrLength))
        protocol = &Protocol::ieee8022llc;
    else
        protocol = ProtocolGroup::getEthertypeProtocolGroup()->getProtocol(typeOrLength);
    auto packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
    packetProtocolTag->setFrontOffset(b(0));
    packetProtocolTag->setBackOffset(b(0));
    packetProtocolTag->setProtocol(protocol);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
}

void Ieee8021qTagEpdHeaderChecker::dropPacket(Packet *packet)
{
    EV_WARN << "Received packet " << packet->getName() << " is not accepted, dropping packet.\n";
    PacketFilterBase::dropPacket(packet, OTHER_PACKET_DROP);
    // TODO or PacketFilterBase::dropPacket(packet, NO_PROTOCOL_FOUND);
}

bool Ieee8021qTagEpdHeaderChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = packet->peekAtFront<Ieee8021qTagEpdHeader>();
    auto typeOrLength = header->getTypeOrLength();
    if (!isIeee8023Length(typeOrLength) && ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(typeOrLength) == nullptr)
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

