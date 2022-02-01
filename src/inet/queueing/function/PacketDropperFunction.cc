//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/function/PacketDropperFunction.h"

#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

Packet *CPacketDropperFunction::selectPacket(IPacketCollection *collection) const
{
    return packetDropperFunction(collection);
}

static Packet *selectPacketAtEnd(IPacketCollection *collection)
{
    return collection->getPacket(collection->getNumPackets() - 1);
}

Register_Packet_Dropper_Function(PacketAtCollectionEndDropper, selectPacketAtEnd);

static Packet *selectPacketAtBegin(IPacketCollection *collection)
{
    return collection->getPacket(0);
}

Register_Packet_Dropper_Function(PacketAtCollectionBeginDropper, selectPacketAtBegin);

static Packet *selectPacketWithLowestOwnerModuleId(IPacketCollection *collection)
{
    Packet *result = nullptr;
    int lowestId = INT_MAX;
    for (int index = 0; index < collection->getNumPackets(); index++) {
        auto packet = collection->getPacket(index);
        auto queue = packet->getOwner();
        auto id = check_and_cast<cModule *>(queue->getOwner())->getId();
        if (id < lowestId) {
            lowestId = id;
            result = packet;
        }
    }
    return result;
}

Register_Packet_Dropper_Function(PacketWithLowestOwnerModuleIdDropper, selectPacketWithLowestOwnerModuleId);

static Packet *selectPacketWithHighestOwnerModuleId(IPacketCollection *collection)
{
    Packet *result = nullptr;
    int highestId = INT_MIN;
    for (int index = 0; index < collection->getNumPackets(); index++) {
        auto packet = collection->getPacket(index);
        auto queue = packet->getOwner();
        auto id = check_and_cast<cModule *>(queue->getOwner())->getId();
        if (id > highestId) {
            highestId = id;
            result = packet;
        }
    }
    return result;
}

Register_Packet_Dropper_Function(PacketWithHighestOwnerModuleIdDropper, selectPacketWithHighestOwnerModuleId);

} // namespace queueing
} // namespace inet

