//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/fragmentation/base/DefragmenterBase.h"

namespace inet {

DefragmenterBase::~DefragmenterBase()
{
    delete defragmentedPacket;
}

void DefragmenterBase::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        deleteSelf = par("deleteSelf");
}

void DefragmenterBase::startDefragmentation(Packet *fragmentPacket)
{
    ASSERT(defragmentedPacket == nullptr);
    std::string name = fragmentPacket->getName();
    auto pos = name.find("-frag");
    if (pos != std::string::npos)
        name = name.substr(0, pos);
    defragmentedPacket = new Packet(name.c_str());
}

void DefragmenterBase::continueDefragmentation(Packet *fragmentPacket)
{
    defragmentedPacket->insertAtBack(fragmentPacket->peekData());
    defragmentedPacket->copyTags(*fragmentPacket);
    expectedFragmentNumber++;
    numProcessedPackets++;
    processedTotalLength += fragmentPacket->getDataLength();
}

void DefragmenterBase::endDefragmentation(Packet *fragmentPacket)
{
    delete fragmentPacket;
    delete defragmentedPacket;
    defragmentedPacket = nullptr;
    expectedFragmentNumber = 0;
    if (deleteSelf)
        deleteModule();
}

void DefragmenterBase::defragmentPacket(Packet *fragmentPacket, bool firstFragment, bool lastFragment, bool expectedFragment)
{
    if (!expectedFragment) {
        EV_INFO << "Ignoring unexpected fragment" << EV_FIELD(packet, *fragmentPacket) << EV_ENDL;
        if (isDefragmenting())
            endDefragmentation(fragmentPacket);
        else
            delete fragmentPacket;
    }
    else {
        if (firstFragment)
            startDefragmentation(fragmentPacket);
        EV_INFO << "Defragmenting packet" << EV_FIELD(packet, *defragmentedPacket) << EV_FIELD(fragment, *fragmentPacket) << EV_ENDL;
        continueDefragmentation(fragmentPacket);
        if (lastFragment) {
            pushOrSendPacket(defragmentedPacket->dup(), outputGate, consumer);
            endDefragmentation(fragmentPacket);
        }
        else
            delete fragmentPacket;
    }
    updateDisplayString();
}

} // namespace inet

