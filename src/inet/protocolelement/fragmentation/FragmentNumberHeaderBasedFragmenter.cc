//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/fragmentation/FragmentNumberHeaderBasedFragmenter.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/fragmentation/header/FragmentNumberHeader_m.h"
#include "inet/protocolelement/fragmentation/tag/FragmentTag_m.h"

namespace inet {

Define_Module(FragmentNumberHeaderBasedFragmenter);

void FragmentNumberHeaderBasedFragmenter::initialize(int stage)
{
    FragmenterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        headerPosition = parseHeaderPosition(par("headerPosition"));
}

Packet *FragmentNumberHeaderBasedFragmenter::createFragmentPacket(Packet *packet, b fragmentOffset, b fragmentLength, int fragmentNumber, int numFragments)
{
    auto fragmentPacket = FragmenterBase::createFragmentPacket(packet, fragmentOffset, fragmentLength, fragmentNumber, numFragments);
    const auto& fragmentHeader = makeShared<FragmentNumberHeader>();
    fragmentHeader->setFragmentNumber(fragmentNumber);
    fragmentHeader->setLastFragment(fragmentNumber == numFragments - 1);
    insertHeader<FragmentNumberHeader>(fragmentPacket, fragmentHeader, headerPosition);
    fragmentPacket->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&AccessoryProtocol::fragmentation);
    return fragmentPacket;
}

} // namespace inet

