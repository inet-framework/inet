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

#include "inet/protocol/fragmentation/base/DefragmenterBase.h"

namespace inet {

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
        EV_INFO << "Ignoring unexpected fragment packet " << fragmentPacket->getName() << "." << endl;
        if (isDefragmenting())
            endDefragmentation(fragmentPacket);
        else
            delete fragmentPacket;
    }
    else {
        if (firstFragment)
            startDefragmentation(fragmentPacket);
        EV_INFO << "Defragmenting packet " << defragmentedPacket->getName() << " from packet " << fragmentPacket->getName() << "." << endl;
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

