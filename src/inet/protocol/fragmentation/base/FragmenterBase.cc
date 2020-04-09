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

#include "inet/common/ModuleAccess.h"
#include "inet/protocol/fragmentation/base/FragmenterBase.h"
#include "inet/protocol/fragmentation/header/FragmentNumberHeader_m.h"

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
    auto fragmentLengths = fragmenterPolicy->computeFragmentLengths(packet);
    b fragmentOffset = b(0);
    int numFragments = fragmentLengths.size();
    for (int fragmentNumber = 0; fragmentNumber < numFragments; fragmentNumber++) {
        auto fragmentLength = fragmentLengths[fragmentNumber];
        auto fragmentPacket = createFragmentPacket(packet, fragmentOffset, fragmentLength, fragmentNumber, numFragments);
        fragmentOffset += fragmentLength;
        EV_INFO << "Fragmenting packet " << packet->getName() << " into packet " << fragmentPacket->getName() << "." << endl;
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

