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

#include "inet/protocol/fragmentation/FragmentTagBasedFragmenter.h"
#include "inet/protocol/fragmentation/tag/FragmentTag_m.h"

namespace inet {

Define_Module(FragmentTagBasedFragmenter);

Packet *FragmentTagBasedFragmenter::createFragmentPacket(Packet *packet, b fragmentOffset, b fragmentLength, int fragmentNumber, int numFragments)
{
    auto fragmentPacket = FragmenterBase::createFragmentPacket(packet, fragmentOffset, fragmentLength, fragmentNumber, numFragments);
    auto fragmentTag = fragmentPacket->addTag<FragmentTag>();
    fragmentTag->setFirstFragment(fragmentNumber == 0);
    fragmentTag->setLastFragment(fragmentNumber == numFragments - 1);
    fragmentTag->setFragmentNumber(fragmentNumber);
    fragmentTag->setNumFragments(numFragments);
    return fragmentPacket;
}

} // namespace inet

