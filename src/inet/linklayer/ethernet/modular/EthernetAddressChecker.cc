//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetAddressChecker.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(EthernetAddressChecker);

void EthernetAddressChecker::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        promiscuous = par("promiscuous");
        interfaceTable.reference(this, "interfaceTableModule", true);
        registerProtocol(Protocol::ethernetMac, nullptr, inputGate);
    }
}

void EthernetAddressChecker::processPacket(Packet *packet)
{
    const auto& header = packet->popAtFront<EthernetMacAddressFields>();
    auto macAddressInd = packet->addTagIfAbsent<MacAddressInd>();
    macAddressInd->setSrcAddress(header->getSrc());
    macAddressInd->setDestAddress(header->getDest());
    auto packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
    packetProtocolTag->setFrontOffset(packetProtocolTag->getFrontOffset() - header->getChunkLength());
}

bool EthernetAddressChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = packet->peekAtFront<EthernetMacAddressFields>();
    auto interfaceInd = packet->getTag<InterfaceInd>();
    auto networkInterface = interfaceTable->getInterfaceById(interfaceInd->getInterfaceId());
    return promiscuous || networkInterface->matchesMacAddress(header->getDest());
}

void EthernetAddressChecker::dropPacket(Packet *packet)
{
    PacketFilterBase::dropPacket(packet, NOT_ADDRESSED_TO_US);
}

} // namespace inet

