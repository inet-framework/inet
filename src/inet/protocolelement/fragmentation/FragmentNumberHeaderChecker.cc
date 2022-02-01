//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/fragmentation/FragmentNumberHeaderChecker.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/fragmentation/header/FragmentNumberHeader_m.h"
#include "inet/protocolelement/fragmentation/tag/FragmentTag_m.h"

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

