//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/fragmentation/FragmentTagBasedDefragmenter.h"

#include "inet/protocolelement/fragmentation/tag/FragmentTag_m.h"

namespace inet {

Define_Module(FragmentTagBasedDefragmenter);

void FragmentTagBasedDefragmenter::pushPacket(Packet *fragmentPacket, cGate *gate)
{
    Enter_Method("pushPacket");
    take(fragmentPacket);
    auto fragmentTag = fragmentPacket->getTag<FragmentTag>();
    bool firstFragment = fragmentTag->getFirstFragment();
    bool lastFragment = fragmentTag->getLastFragment();
    bool expectedFragment = fragmentTag->getFragmentNumber() == -1 || fragmentTag->getFragmentNumber() == expectedFragmentNumber;
    defragmentPacket(fragmentPacket, firstFragment, lastFragment, expectedFragment);
}

} // namespace inet

