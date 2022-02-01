//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/fragmentation/FragmentNumberHeaderInserter.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/fragmentation/header/FragmentNumberHeader_m.h"
#include "inet/protocolelement/fragmentation/tag/FragmentTag_m.h"

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
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&AccessoryProtocol::fragmentation);
}

} // namespace inet

