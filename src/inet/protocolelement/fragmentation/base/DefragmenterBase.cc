//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
    if (stage == INITSTAGE_LOCAL) {
        deleteSelf = par("deleteSelf");
        headerLength = b(par("headerLength"));
        footerLength = b(par("footerLength"));
    }
}

void DefragmenterBase::startDefragmentation(Packet *fragmentPacket)
{
    ASSERT(defragmentedPacket == nullptr);
    std::string name = fragmentPacket->getName();
    auto pos = name.find("-frag");
    if (pos != std::string::npos)
        name = name.substr(0, pos);
    defragmentedPacket = new Packet(name.c_str());
    // add header
    if (headerLength > b(0))
        defragmentedPacket->insertAtFront(fragmentPacket->peekDataAt(b(0), headerLength));
    // add footer
    if (footerLength > b(0))
        defragmentedPacket->insertAtBack(fragmentPacket->peekDataAt(defragmentedPacket->getDataLength() - footerLength, footerLength));
}

void DefragmenterBase::continueDefragmentation(Packet *fragmentPacket)
{
    // remove old footer
    if (footerLength > b(0))
        defragmentedPacket->removeAtBack(footerLength);
    // add body and new footer
    defragmentedPacket->insertAtBack(fragmentPacket->peekDataAt(headerLength, fragmentPacket->getDataLength() - headerLength));

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

