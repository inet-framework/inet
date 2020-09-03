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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/protocol/common/AccessoryProtocol.h"
#include "inet/protocol/selectivity/DestinationMacAddressHeader_m.h"
#include "inet/protocol/selectivity/ReceiveAtMacAddress.h"

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

