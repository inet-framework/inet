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

#include "inet/protocolelement/selectivity/ReceiveAtMacAddress.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/selectivity/DestinationMacAddressHeader_m.h"

namespace inet {

Define_Module(ReceiveAtMacAddress);

void ReceiveAtMacAddress::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        address = MacAddress(par("address").stringValue());
        registerService(AccessoryProtocol::destinationMacAddress, nullptr, inputGate);
        registerProtocol(AccessoryProtocol::destinationMacAddress, nullptr, outputGate);
        getContainingNicModule(this)->setMacAddress(address);
    }
}

void ReceiveAtMacAddress::processPacket(Packet *packet)
{
    packet->popAtFront<DestinationMacAddressHeader>();
    // TODO: KLUDGE:
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&AccessoryProtocol::sequenceNumber);
}

bool ReceiveAtMacAddress::matchesPacket(const Packet *packet) const
{
    auto header = packet->peekAtFront<DestinationMacAddressHeader>();
    return header->getDestinationAddress() == address;
}

} // namespace inet

