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

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/protocol/ethernet/EthernetPreambleChecker.h"

namespace inet {

Define_Module(EthernetPreambleChecker);

bool EthernetPreambleChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = packet->popAtFront<EthernetPhyHeader>(b(-1), Chunk::PF_ALLOW_INCORRECT + Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
    return header->isCorrect() && header->isProperlyRepresented();
}

void EthernetPreambleChecker::dropPacket(Packet *packet)
{
    PacketFilterBase::dropPacket(packet, INCORRECTLY_RECEIVED);
}

} // namespace inet

