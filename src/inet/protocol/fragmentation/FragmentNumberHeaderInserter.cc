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
#include "inet/protocol/contract/IProtocol.h"
#include "inet/protocol/fragmentation/FragmentNumberHeaderInserter.h"
#include "inet/protocol/fragmentation/header/FragmentNumberHeader_m.h"
#include "inet/protocol/fragmentation/tag/FragmentTag_m.h"

namespace inet {

Define_Module(FragmentNumberHeaderInserter);

void FragmentNumberHeaderInserter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        headerPosition = parseHeaderPosition(par("headerPosition"));
}

void FragmentNumberHeaderInserter::processPacket(Packet *packet)
{
    auto fragmentTag = packet->getTag<FragmentTag>();
    const auto& fragmentHeader = makeShared<FragmentNumberHeader>();
    fragmentHeader->setFragmentNumber(fragmentTag->getFragmentNumber());
    fragmentHeader->setLastFragment(fragmentTag->getLastFragment());
    insertHeader<FragmentNumberHeader>(packet, fragmentHeader, headerPosition);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&IProtocol::fragmentation);
}

} // namespace inet

