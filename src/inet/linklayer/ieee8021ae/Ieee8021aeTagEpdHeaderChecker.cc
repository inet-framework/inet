//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021ae/Ieee8021aeTagEpdHeaderChecker.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/ieee8021ae/Ieee8021aeTagHeader_m.h"

namespace inet {

Define_Module(Ieee8021aeTagEpdHeaderChecker);

void Ieee8021aeTagEpdHeaderChecker::processPacket(Packet *packet)
{
    // TODO this code is incomplete
    const auto& header = packet->popAtFront<Ieee8021aeTagEpdHeader>();
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

void Ieee8021aeTagEpdHeaderChecker::dropPacket(Packet *packet)
{
    EV_WARN << "Dropping packet because protocol is not found" << EV_FIELD(packet) << EV_ENDL;
    PacketFilterBase::dropPacket(packet, NO_PROTOCOL_FOUND);
}

bool Ieee8021aeTagEpdHeaderChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = packet->peekAtFront<Ieee8021aeTagEpdHeader>();
    auto typeOrLength = header->getTypeOrLength();
    if (!isIeee8023Length(typeOrLength) && ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(typeOrLength) == nullptr)
        return false;
    else
        return true;
}

} // namespace inet

