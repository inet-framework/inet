//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

