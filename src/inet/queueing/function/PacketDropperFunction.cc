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

#include "inet/common/Simsignals.h"
#include "inet/queueing/function/PacketDropperFunction.h"

namespace inet {
namespace queueing {

Packet *CPacketDropperFunction::selectPacket(IPacketCollection* collection) const
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

