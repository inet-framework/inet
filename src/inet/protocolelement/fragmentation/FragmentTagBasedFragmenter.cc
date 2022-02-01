//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/fragmentation/FragmentTagBasedFragmenter.h"

#include "inet/protocolelement/fragmentation/tag/FragmentTag_m.h"

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

