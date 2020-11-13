//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/linklayer/ethernet/modular/EthernetMacHeaderChecker.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(EthernetMacHeaderChecker);

void EthernetMacHeaderChecker::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        promiscuous = par("promiscuous");
        interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    }
    else if (stage == INITSTAGE_LINK_LAYER)
        registerProtocol(Protocol::ethernetMac, nullptr, inputGate);
}

void EthernetMacHeaderChecker::processPacket(Packet *packet)
{
    const auto& header = packet->popAtFront<EthernetMacHeader>();
    auto macAddressInd = packet->addTagIfAbsent<MacAddressInd>();
    macAddressInd->setSrcAddress(header->getSrc());
    macAddressInd->setDestAddress(header->getDest());
    auto packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
    auto typeOrLength = header->getTypeOrLength();
    const Protocol *protocol;
    if (isIeee8023Length(typeOrLength))
        protocol = &Protocol::ieee8022llc;
    else
        protocol = ProtocolGroup::ethertype.getProtocol(typeOrLength);
    packetProtocolTag->setFrontOffset(b(0));
    packetProtocolTag->setBackOffset(b(0));
    packetProtocolTag->setProtocol(protocol);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
}

bool EthernetMacHeaderChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = packet->peekAtFront<EthernetMacHeader>();
    auto interfaceInd = packet->getTag<InterfaceInd>();
    auto networkInterface = interfaceTable->getInterfaceById(interfaceInd->getInterfaceId());
    if (!promiscuous && !networkInterface->matchesMacAddress(header->getDest()))
        return false;
    else {
        auto typeOrLength = header->getTypeOrLength();
        if (isIeee8023Length(typeOrLength))
            return true;
        else
            return ProtocolGroup::ethertype.findProtocol(typeOrLength) != nullptr;
    }
}

void EthernetMacHeaderChecker::dropPacket(Packet *packet)
{
    // TODO: or PacketFilterBase::dropPacket(packet, NO_PROTOCOL_FOUND);
    PacketFilterBase::dropPacket(packet, NOT_ADDRESSED_TO_US);
}

} // namespace inet

