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
#include "inet/protocol/ethernet/EthernetFragmentPreambleChecker.h"
#include "inet/protocol/fragmentation/tag/FragmentTag_m.h"

namespace inet {

Define_Module(EthernetFragmentPreambleChecker);

bool EthernetFragmentPreambleChecker::matchesPacket(Packet *packet)
{
    const auto& header = packet->popAtFront<EthernetFragmentPhyHeader>(b(-1), Chunk::PF_ALLOW_INCORRECT + Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
    if (header->isIncorrect() || header->isImproperlyRepresented())
        return false;
    else if (header->getPreambleType() != SMD_Sx && header->getPreambleType() != SMD_Cx)
        return false;
    else {
        auto fragmentTag = packet->addTag<FragmentTag>();
        fragmentTag->setFragmentNumber(-1);
        fragmentTag->setNumFragments(-1);
        if (header->getSmdNumber() == smdNumber) {
            if (header->getFragmentNumber() == fragmentNumber) {
                if (header->getPreambleType() == SMD_Cx)
                    fragmentNumber = (fragmentNumber + 1) % 4;
                fragmentTag->setFirstFragment(false);
                return true;
            }
            else {
                smdNumber = -1;
                return false;
            }
        }
        else {
            if (header->getFragmentNumber() == 0) {
                smdNumber = header->getSmdNumber();
                fragmentNumber = 0;
                fragmentTag->setFirstFragment(true);
                return true;
            }
            else {
                smdNumber = -1;
                return false;
            }
        }
    }
}

void EthernetFragmentPreambleChecker::dropPacket(Packet *packet)
{
    PacketFilterBase::dropPacket(packet, INCORRECTLY_RECEIVED);
}

} // namespace inet

