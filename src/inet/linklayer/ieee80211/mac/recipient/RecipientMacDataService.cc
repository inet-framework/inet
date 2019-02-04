//
// Copyright (C) 2016 OpenSim Ltd.
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

#include "inet/common/Simsignals.h"
#include "inet/linklayer/ieee80211/mac/duplicateremoval/LegacyDuplicateRemoval.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/BasicReassembly.h"
#include "inet/linklayer/ieee80211/mac/recipient/RecipientMacDataService.h"

namespace inet {
namespace ieee80211 {

Define_Module(RecipientMacDataService);

void RecipientMacDataService::initialize()
{
    duplicateRemoval = new LegacyDuplicateRemoval();
    basicReassembly = new BasicReassembly();
}

Packet *RecipientMacDataService::defragment(Packet *dataOrMgmtFrame)
{
    Packet *packet = basicReassembly->addFragment(dataOrMgmtFrame);
    if (packet && packet->peekAtFront<Ieee80211DataOrMgmtHeader>()) {
        emit(packetDefragmentedSignal, packet);
        return packet;
    }
    else
        return nullptr;
}

std::vector<Packet *> RecipientMacDataService::dataOrMgmtFrameReceived(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    if (duplicateRemoval && duplicateRemoval->isDuplicate(header)) {
        EV_WARN << "Dropping duplicate packet " << *packet << ".\n";
        PacketDropDetails details;
        details.setReason(DUPLICATE_DETECTED);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return std::vector<Packet *>();
    }
    Packet *defragmentedFrame = nullptr;
    if (basicReassembly) { // FIXME: defragmentation
        defragmentedFrame = defragment(packet);
    }
    return defragmentedFrame != nullptr ? std::vector<Packet *>({defragmentedFrame}) : std::vector<Packet *>();
}

std::vector<Packet *> RecipientMacDataService::dataFrameReceived(Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader)
{
    return dataOrMgmtFrameReceived(dataPacket, dataHeader);
}

std::vector<Packet *> RecipientMacDataService::managementFrameReceived(Packet *mgmtPacket, const Ptr<const Ieee80211MgmtHeader>& mgmtHeader)
{
    return dataOrMgmtFrameReceived(mgmtPacket, mgmtHeader);
}

std::vector<Packet *> RecipientMacDataService::controlFrameReceived(Packet *controlPacket, const Ptr<const Ieee80211MacHeader>& controlHeader)
{
    return std::vector<Packet *>(); // has nothing to do
}

RecipientMacDataService::~RecipientMacDataService()
{
    delete duplicateRemoval;
    delete basicReassembly;
}

} /* namespace ieee80211 */
} /* namespace inet */
