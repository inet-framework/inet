//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetTypeOrLengthChecker.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(EthernetTypeOrLengthChecker);

void EthernetTypeOrLengthChecker::processPacket(Packet *packet)
{
    const auto& header = packet->popAtFront<EthernetTypeOrLengthField>();
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

bool EthernetTypeOrLengthChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = packet->peekAtFront<EthernetTypeOrLengthField>();
    auto typeOrLength = header->getTypeOrLength();
    if (isIeee8023Length(typeOrLength))
        return true;
    auto protocol = ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(typeOrLength);
    return protocol != nullptr;
}

void EthernetTypeOrLengthChecker::dropPacket(Packet *packet)
{
    PacketFilterBase::dropPacket(packet, NO_PROTOCOL_FOUND);
}

} // namespace inet

