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

#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/queueing/function/PacketComparatorFunction.h"

namespace inet {

static int comparePacketUserPriorityInd(Packet *packet1, Packet *packet2)
{
    const auto& userPriorityInd1 = packet1->findTag<UserPriorityInd>();
    auto userPriority1 = userPriorityInd1 != nullptr ? userPriorityInd1->getUserPriority() : 0;
    const auto& userPriorityInd2 = packet2->findTag<UserPriorityInd>();
    auto userPriority2 = userPriorityInd2 != nullptr ? userPriorityInd2->getUserPriority() : 0;
    return userPriority2 - userPriority1;
}

Register_Packet_Comparator_Function(PacketUserPriorityIndComparator, comparePacketUserPriorityInd);

} // namespace inet

