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

#include "inet/queueing/PacketDropperFunction.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

static bool isOverloaded(IPacketCollection *collection)
{
    auto maxNumPackets = collection->getMaxNumPackets();
    auto maxTotalLength = collection->getMaxTotalLength();
    return (maxNumPackets != -1 && collection->getNumPackets() > maxNumPackets) ||
           (maxTotalLength != b(-1) && collection->getTotalLength() > maxTotalLength);
}

static void dropPacket(IPacketCollection *collection, int i)
{
    auto packet = collection->getPacket(i);
    collection->removePacket(packet);
    PacketDropDetails details;
    details.setReason(QUEUE_OVERFLOW);
    details.setLimit(collection->getMaxNumPackets());
    check_and_cast<cModule *>(collection)->emit(packetDroppedSignal, packet, &details);
    delete packet;
}

void CPacketDropperFunction::dropPackets(IPacketCollection* collection) const
{
    while (!collection->isEmpty() && isOverloaded(collection))
        packetDropperFunction(collection);
}

static void dropPacketAtEnd(IPacketCollection *collection)
{
    dropPacket(collection, collection->getNumPackets() - 1);
}

Register_Packet_Dropper_Function(PacketAtCollectionEndDropper, dropPacketAtEnd);

static void dropPacketAtBegin(IPacketCollection *collection)
{
    dropPacket(collection, 0);
}

Register_Packet_Dropper_Function(PacketAtCollectionBeginDropper, dropPacketAtBegin);

static void dropPacketWithLowestOwnerModuleId(IPacketCollection *collection)
{
    int lowestIdIndex = 0;
    int lowestId = check_and_cast<cModule *>(collection->getPacket(lowestIdIndex)->getOwner()->getOwner())->getId();
    for (int index = 0; index < collection->getNumPackets(); index++) {
        auto id = check_and_cast<cModule *>(collection->getPacket(index)->getOwner()->getOwner())->getId();
        if (id < lowestId)
            lowestIdIndex = index;
    }
    dropPacket(collection, lowestIdIndex);
}

Register_Packet_Dropper_Function(PacketWithLowestOwnerModuleIdDropper, dropPacketWithLowestOwnerModuleId);

static void dropPacketWithHighestOwnerModuleId(IPacketCollection *collection)
{
    int highestIdIndex = 0;
    int highestId = check_and_cast<cModule *>(collection->getPacket(highestIdIndex)->getOwner()->getOwner())->getId();
    for (int index = 0; index < collection->getNumPackets(); index++) {
        auto id = check_and_cast<cModule *>(collection->getPacket(index)->getOwner()->getOwner())->getId();
        if (id > highestId)
            highestIdIndex = index;
    }
    dropPacket(collection, highestIdIndex);
}

Register_Packet_Dropper_Function(PacketWithHighestOwnerModuleIdDropper, dropPacketWithHighestOwnerModuleId);

} // namespace queueing
} // namespace inet

