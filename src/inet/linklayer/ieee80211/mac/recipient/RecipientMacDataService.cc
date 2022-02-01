//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/recipient/RecipientMacDataService.h"

#include "inet/common/Simsignals.h"
#include "inet/linklayer/ieee80211/mac/duplicateremoval/LegacyDuplicateRemoval.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/BasicReassembly.h"

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
    if (basicReassembly) { // FIXME defragmentation
        defragmentedFrame = defragment(packet);
    }
    return defragmentedFrame != nullptr ? std::vector<Packet *>({ defragmentedFrame }) : std::vector<Packet *>();
}

std::vector<Packet *> RecipientMacDataService::dataFrameReceived(Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader)
{
    Enter_Method("dataFrameReceived");
    take(dataPacket);
    return dataOrMgmtFrameReceived(dataPacket, dataHeader);
}

std::vector<Packet *> RecipientMacDataService::managementFrameReceived(Packet *mgmtPacket, const Ptr<const Ieee80211MgmtHeader>& mgmtHeader)
{
    Enter_Method("managementFrameReceived");
    take(mgmtPacket);
    return dataOrMgmtFrameReceived(mgmtPacket, mgmtHeader);
}

std::vector<Packet *> RecipientMacDataService::controlFrameReceived(Packet *controlPacket, const Ptr<const Ieee80211MacHeader>& controlHeader)
{
    Enter_Method("controlFrameReceived");
    return std::vector<Packet *>(); // has nothing to do
}

RecipientMacDataService::~RecipientMacDataService()
{
    delete duplicateRemoval;
    delete basicReassembly;
}

} /* namespace ieee80211 */
} /* namespace inet */

