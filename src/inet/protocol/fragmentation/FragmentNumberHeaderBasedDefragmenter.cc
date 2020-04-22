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

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/protocol/contract/IProtocol.h"
#include "inet/protocol/fragmentation/FragmentNumberHeaderBasedDefragmenter.h"
#include "inet/protocol/fragmentation/header/FragmentNumberHeader_m.h"

namespace inet {

Define_Module(FragmentNumberHeaderBasedDefragmenter);

void FragmentNumberHeaderBasedDefragmenter::initialize(int stage)
{
    DefragmenterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        headerPosition = parseHeaderPosition(par("headerPosition"));
        registerService(IProtocol::fragmentation, nullptr, inputGate);
        registerProtocol(IProtocol::fragmentation, nullptr, outputGate);
    }
}

void FragmentNumberHeaderBasedDefragmenter::pushPacket(Packet *fragmentPacket, cGate *gate)
{
    Enter_Method("pushPacket");
    take(fragmentPacket);
    const auto& fragmentHeader = popHeader<FragmentNumberHeader>(fragmentPacket, headerPosition, B(1));
    bool firstFragment = fragmentHeader->getFragmentNumber() == 0;
    bool lastFragment = fragmentHeader->getLastFragment();
    bool expectedFragment = fragmentHeader->getFragmentNumber() == expectedFragmentNumber;
    defragmentPacket(fragmentPacket, firstFragment, lastFragment, expectedFragment);
}

} // namespace inet

