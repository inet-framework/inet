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
#include "inet/protocol/common/AccessoryProtocol.h"
#include "inet/protocol/fragmentation/FragmentNumberHeaderChecker.h"
#include "inet/protocol/fragmentation/header/FragmentNumberHeader_m.h"
#include "inet/protocol/fragmentation/tag/FragmentTag_m.h"

namespace inet {

Define_Module(FragmentNumberHeaderChecker);

void FragmentNumberHeaderChecker::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        headerPosition = parseHeaderPosition(par("headerPosition"));
        registerService(AccessoryProtocol::fragmentation, nullptr, inputGate);
        registerProtocol(AccessoryProtocol::fragmentation, nullptr, outputGate);
    }
}

void FragmentNumberHeaderChecker::processPacket(Packet *packet)
{
    auto fragmentTag = packet->addTagIfAbsent<FragmentTag>();
    const auto& fragmentHeader = popHeader<FragmentNumberHeader>(packet, headerPosition, B(1));
    fragmentTag->setFirstFragment(fragmentHeader->getFragmentNumber() == 0);
    fragmentTag->setLastFragment(fragmentHeader->getLastFragment());
    fragmentTag->setFragmentNumber(fragmentHeader->getFragmentNumber());
    fragmentTag->setNumFragments(-1);
}

} // namespace inet

