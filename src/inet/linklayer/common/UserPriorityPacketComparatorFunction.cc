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

#include "inet/linklayer/common/UserPriority.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/queueing/function/PacketComparatorFunction.h"

namespace inet {

static int getPacketPriority(cObject *object)
{
    auto packet = static_cast<Packet *>(object);
    int up = packet->getTag<UserPriorityReq>()->getUserPriority();
    return (up == UP_BK) ? -2 : (up == UP_BK2) ? -1 : up;  // because UP_BE==0, but background traffic should have lower priority than best effort
}

static int comparePacketsByUserPriority(Packet *a, Packet *b)
{
    return getPacketPriority(b) - getPacketPriority(a);
}

Register_Packet_Comparator_Function(PacketUserPriorityComparator, comparePacketsByUserPriority);

} // namespace inet

