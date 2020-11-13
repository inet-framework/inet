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

#include "inet/physicallayer/wired/ethernet/EthernetFragmentPhyHeaderChecker.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"
#include "inet/protocolelement/fragmentation/tag/FragmentTag_m.h"

namespace inet {

namespace physicallayer {

Define_Module(EthernetFragmentPhyHeaderChecker);

bool EthernetFragmentPhyHeaderChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = packet->peekAtFront<EthernetFragmentPhyHeader>(b(-1), Chunk::PF_ALLOW_INCORRECT + Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED);
    if (header->isIncorrect() || header->isImproperlyRepresented())
        return false;
    else if (header->getPreambleType() != SMD_Sx && header->getPreambleType() != SMD_Cx)
        return false;
    else {
        if (header->getSmdNumber() == smdNumber)
            return header->getFragmentNumber() == fragmentNumber % 4;
        else
            return header->getFragmentNumber() == 0;
    }
}

void EthernetFragmentPhyHeaderChecker::processPacket(Packet *packet)
{
    const auto& header = packet->popAtFront<EthernetFragmentPhyHeader>();
    const auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
    packetProtocolTag->setFrontOffset(packetProtocolTag->getFrontOffset() - header->getChunkLength());
    const auto& dispatchProtocolReq = packet->addTagIfAbsent<DispatchProtocolReq>();
    dispatchProtocolReq->setProtocol(&Protocol::ethernetMac);
    dispatchProtocolReq->setServicePrimitive(SP_INDICATION);
    auto fragmentTag = packet->addTag<FragmentTag>();
    if (header->getSmdNumber() == smdNumber) {
        ASSERT(header->getFragmentNumber() == fragmentNumber % 4);
        fragmentTag->setFirstFragment(false);
        fragmentNumber++;
    }
    else {
        ASSERT(header->getFragmentNumber() == 0);
        fragmentTag->setFirstFragment(true);
        smdNumber = header->getSmdNumber();
        fragmentNumber = 0;
    }
    fragmentTag->setFragmentNumber(fragmentNumber);
    fragmentTag->setNumFragments(-1);
}

void EthernetFragmentPhyHeaderChecker::dropPacket(Packet *packet)
{
    PacketFilterBase::dropPacket(packet, INCORRECTLY_RECEIVED);
}

} // namespace physicallayer

} // namespace inet

