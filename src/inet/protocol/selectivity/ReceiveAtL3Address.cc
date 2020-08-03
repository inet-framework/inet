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
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/protocol/contract/IProtocol.h"
#include "inet/protocol/selectivity/DestinationL3AddressHeader_m.h"
#include "inet/protocol/selectivity/ReceiveAtL3Address.h"

namespace inet {

Define_Module(ReceiveAtL3Address);

void ReceiveAtL3Address::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        address = Ipv4Address(par("address").stringValue());
        registerService(IProtocol::destinationL3Address, nullptr, inputGate);
        registerProtocol(IProtocol::destinationL3Address, nullptr, outputGate);
    }
}

void ReceiveAtL3Address::processPacket(Packet *packet)
{
    packet->popAtFront<DestinationL3AddressHeader>();
}

bool ReceiveAtL3Address::matchesPacket(const Packet *packet) const
{
    auto header = packet->peekAtFront<DestinationL3AddressHeader>();
    return header->getDestinationAddress() == address;
}

} // namespace inet

