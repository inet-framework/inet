//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021r/Ieee8021rTagEpdHeaderChecker.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/ProtocolUtils.h"
#include "inet/common/SequenceNumberTag_m.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/ieee8021r/Ieee8021rTagHeader_m.h"

namespace inet {

Define_Module(Ieee8021rTagEpdHeaderChecker);

void Ieee8021rTagEpdHeaderChecker::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LINK_LAYER)
        registerProtocol(Protocol::ieee8021rTag, nullptr, inputGate);
}

void Ieee8021rTagEpdHeaderChecker::processPacket(Packet *packet)
{
    const auto& header = packet->popAtFront<Ieee8021rTagEpdHeader>();
    appendEncapsulationProtocolInd(packet, &Protocol::ieee8021rTag);
    packet->addTagIfAbsent<SequenceNumberInd>()->setSequenceNumber(header->getSequenceNumber());
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

void Ieee8021rTagEpdHeaderChecker::dropPacket(Packet *packet)
{
    EV_WARN << "Received packet " << packet->getName() << " is not accepted, dropping packet.\n";
    PacketFilterBase::dropPacket(packet, OTHER_PACKET_DROP);
    // TODO or PacketFilterBase::dropPacket(packet, NO_PROTOCOL_FOUND);
}

bool Ieee8021rTagEpdHeaderChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = packet->peekAtFront<Ieee8021rTagEpdHeader>();
    auto typeOrLength = header->getTypeOrLength();
    return isIeee8023Length(typeOrLength) || ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(typeOrLength) != nullptr;
}

} // namespace inet

