//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/common/UserPriority.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/queueing/function/PacketComparatorFunction.h"

namespace inet {

static int getPacketPriority(cObject *object)
{
    auto packet = static_cast<Packet *>(object);
    int up = packet->getTag<UserPriorityReq>()->getUserPriority();
    return (up == UP_BK) ? -2 : (up == UP_BK2) ? -1 : up; // because UP_BE==0, but background traffic should have lower priority than best effort
}

static int comparePacketsByUserPriority(Packet *a, Packet *b)
{
    return getPacketPriority(b) - getPacketPriority(a);
}

Register_Packet_Comparator_Function(PacketUserPriorityComparator, comparePacketsByUserPriority);

} // namespace inet

