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

#include "inet/physicallayer/wired/ethernet/EthernetPhyHeaderChecker.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"

namespace inet {

namespace physicallayer {

Define_Module(EthernetPhyHeaderChecker);

void EthernetPhyHeaderChecker::processPacket(Packet *packet)
{
    packet->popAtFront<EthernetPhyHeader>(b(-1), Chunk::PF_ALLOW_INCORRECT + Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED);
    const auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
    packetProtocolTag->setProtocol(&Protocol::ethernetMac);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
    const auto& dispatchProtocolReq = packet->addTagIfAbsent<DispatchProtocolReq>();
    dispatchProtocolReq->setProtocol(&Protocol::ethernetMac);
    dispatchProtocolReq->setServicePrimitive(SP_INDICATION);
}

bool EthernetPhyHeaderChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = packet->peekAtFront<EthernetPhyHeader>(b(-1), Chunk::PF_ALLOW_INCORRECT + Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED);
    return header->isCorrect() && header->isProperlyRepresented();
}

void EthernetPhyHeaderChecker::dropPacket(Packet *packet)
{
    PacketFilterBase::dropPacket(packet, INCORRECTLY_RECEIVED);
}

} // namespace physicallayer

} // namespace inet

