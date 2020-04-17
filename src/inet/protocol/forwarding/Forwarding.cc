//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/protocol/forwarding/Forwarding.h"
#include "inet/protocol/contract/IProtocol.h"
#include "inet/protocol/selectivity/DestinationL3AddressHeader_m.h"

namespace inet {

Define_Module(Forwarding);

void Forwarding::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *addressAsString = par("address");
        if (strlen(addressAsString) != 0)
            address = L3AddressResolver().resolve(addressAsString);
        registerService(IProtocol::forwarding, inputGate, inputGate);
        registerProtocol(IProtocol::forwarding, outputGate, outputGate);
    }
}

std::pair<L3Address, int> Forwarding::findNextHop(const L3Address& destinationAddress)
{
    // TODO: KLUDGE: this is for testing only
    if (destinationAddress == Ipv4Address("10.0.0.10")) {
        if (address == Ipv4Address("10.0.0.1"))
            return {Ipv4Address("10.0.0.2"), 0};
        else if (address == Ipv4Address("10.0.0.2"))
            return {Ipv4Address("10.0.0.7"), 2};
        else if (address == Ipv4Address("10.0.0.7"))
            return {Ipv4Address("10.0.0.10"), 2};
    }
    return {Ipv4Address(), -1};
}

void Forwarding::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    packet->removeTagIfPresent<DispatchProtocolReq>();
    auto header = packet->peekAtFront<DestinationL3AddressHeader>();
    auto nextHop = findNextHop(header->getDestinationAddress());
    auto nextHopAddress = nextHop.first;
    auto interfaceIndex = nextHop.second;
    if (nextHopAddress.isUnspecified()) {
        packet->popAtFront<DestinationL3AddressHeader>();
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&IProtocol::destinationL3Address);
        pushOrSendPacket(packet, outputGate, consumer);
    }
    else {
        auto interface = check_and_cast<InterfaceEntry *>(getParentModule()->getParentModule()->getSubmodule("interface", interfaceIndex));
        packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(interface->getInterfaceId());
        // TODO: KLUDGE: this is for testing only
        MacAddress destinationMacAddress("00-AA-00-00-00-00");
        destinationMacAddress.setAddressByte(5, nextHopAddress.toIpv4().getDByte(3));
        packet->addTagIfAbsent<MacAddressReq>()->setDestAddress(destinationMacAddress);
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&IProtocol::forwarding);
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&IProtocol::hopLimit);
        packet->trimFront();
        pushOrSendPacket(packet, outputGate, consumer);
    }
}

} // namespace inet

