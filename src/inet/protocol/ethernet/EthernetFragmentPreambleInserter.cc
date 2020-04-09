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
#include "inet/protocol/ethernet/EthernetFragmentPreambleInserter.h"
#include "inet/protocol/fragmentation/tag/FragmentTag_m.h"

namespace inet {

Define_Module(EthernetFragmentPreambleInserter);

void EthernetFragmentPreambleInserter::processPacket(Packet *packet)
{
    auto fragmentTag = packet->getTag<FragmentTag>();
    const auto& header = makeShared<EthernetFragmentPhyHeader>();
    header->setPreambleType(fragmentTag->getFirstFragment() ? SMD_Sx : SMD_Cx);
    header->setSmdNumber(smdNumber);
    header->setFragmentNumber(fragmentNumber);
    packet->insertAtFront(header);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetPhy);
    if (!fragmentTag->getFirstFragment())
        fragmentNumber = (fragmentNumber + 1) % 4;
    if (fragmentTag->getLastFragment()) {
        fragmentNumber = 0;
        smdNumber = (smdNumber + 1) % 4;
    }
}

} // namespace inet

