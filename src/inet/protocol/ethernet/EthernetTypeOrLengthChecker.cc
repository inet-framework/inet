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

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/protocol/ethernet/EthernetTypeOrLengthChecker.h"

namespace inet {

Define_Module(EthernetTypeOrLengthChecker);

void EthernetTypeOrLengthChecker::processPacket(Packet *packet)
{
    const auto& header = packet->popAtFront<Ieee8023TypeOrLength>();
    auto protocol = ProtocolGroup::ethertype.getProtocol(header->getTypeOrLength());
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(protocol);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
    // TODO: move to interface entry
    auto interfaceEntry = check_and_cast<InterfaceEntry *>(getParentModule());
    packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
}

bool EthernetTypeOrLengthChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = packet->peekAtFront<Ieee8023TypeOrLength>();
    auto protocol = ProtocolGroup::ethertype.findProtocol(header->getTypeOrLength());
    return protocol != nullptr;
}

void EthernetTypeOrLengthChecker::dropPacket(Packet *packet)
{
    PacketFilterBase::dropPacket(packet, NO_PROTOCOL_FOUND);
}

} // namespace inet

