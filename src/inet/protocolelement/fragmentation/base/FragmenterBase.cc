//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/fragmentation/base/FragmenterBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/protocolelement/fragmentation/header/FragmentNumberHeader_m.h"

namespace inet {

void FragmenterBase::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        deleteSelf = par("deleteSelf");
        const char *fragmenterPolicyClass = par("fragmenterPolicyClass");
        if (*fragmenterPolicyClass != '\0')
            fragmenterPolicy = createFragmenterPolicy(fragmenterPolicyClass);
        else
            fragmenterPolicy = findModuleFromPar<IFragmenterPolicy>(par("fragmenterPolicyModule"), this);
    }
}

IFragmenterPolicy *FragmenterBase::createFragmenterPolicy(const char *fragmenterPolicyClass) const
{
    return check_and_cast<IFragmenterPolicy *>(createOne(fragmenterPolicyClass));
}

Packet *FragmenterBase::createFragmentPacket(Packet *packet, b fragmentOffset, b fragmentLength, int fragmentNumber, int numFragments)
{
    std::string name = std::string(packet->getName()) + "-frag" + std::to_string(fragmentNumber);
    auto fragmentPacket = new Packet(name.c_str());
    auto fragmentData = packet->peekDataAt(fragmentOffset, fragmentLength);
    fragmentPacket->copyTags(*packet);
    fragmentPacket->insertAtBack(fragmentData);
    return fragmentPacket;
}

void FragmenterBase::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    auto fragmentLengths = fragmenterPolicy->computeFragmentLengths(packet);
    b fragmentOffset = b(0);
    int numFragments = fragmentLengths.size();
    for (int fragmentNumber = 0; fragmentNumber < numFragments; fragmentNumber++) {
        auto fragmentLength = fragmentLengths[fragmentNumber];
        auto fragmentPacket = createFragmentPacket(packet, fragmentOffset, fragmentLength, fragmentNumber, numFragments);
        fragmentOffset += fragmentLength;
        EV_INFO << "Fragmenting packet" << EV_FIELD(packet) << EV_FIELD(fragment, *fragmentPacket) << EV_ENDL;
        pushOrSendPacket(fragmentPacket, outputGate, consumer);
    }
    processedTotalLength += packet->getDataLength();
    numProcessedPackets++;
    updateDisplayString();
    if (deleteSelf)
        deleteModule();
    delete packet;
}

} // namespace inet

