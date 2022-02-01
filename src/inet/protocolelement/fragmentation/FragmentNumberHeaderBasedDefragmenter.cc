//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/fragmentation/FragmentNumberHeaderBasedDefragmenter.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/fragmentation/header/FragmentNumberHeader_m.h"

namespace inet {

Define_Module(FragmentNumberHeaderBasedDefragmenter);

void FragmentNumberHeaderBasedDefragmenter::initialize(int stage)
{
    DefragmenterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        headerPosition = parseHeaderPosition(par("headerPosition"));
        registerService(AccessoryProtocol::fragmentation, nullptr, inputGate);
        registerProtocol(AccessoryProtocol::fragmentation, nullptr, outputGate);
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

