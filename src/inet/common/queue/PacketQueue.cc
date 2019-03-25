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

#include "inet/common/queue/PacketQueue.h"

namespace inet {

PacketQueue::PacketQueue(const char *name) :
    cPacketQueue(name)
{
}

void PacketQueue::checkInsertion(cPacket *packet)
{
    if ((maxPacketLength != -1 && getLength() == maxPacketLength) ||
        (maxBitLength != -1 && getBitLength() + packet->getBitLength() > maxBitLength))
        throw cRuntimeError("Queue is full");
}

void PacketQueue::insert(cPacket *packet)
{
    checkInsertion(packet);
    cQueue::insert(packet);
}

void PacketQueue::insertBefore(cPacket *where, cPacket *packet)
{
    checkInsertion(packet);
    cQueue::insertBefore(where, packet);
}

void PacketQueue::insertAfter(cPacket *where, cPacket *packet)
{
    checkInsertion(packet);
    cQueue::insertAfter(where, packet);
}

} // namespace ieee80211
