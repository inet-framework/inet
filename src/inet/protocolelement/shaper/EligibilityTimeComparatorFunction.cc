//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/shaper/EligibilityTimeTag_m.h"
#include "inet/queueing/function/PacketComparatorFunction.h"

namespace inet {

static clocktime_t getPacketEligibilityTime(cObject *object)
{
    auto packet = static_cast<Packet *>(object);
    return packet->getTag<EligibilityTimeTag>()->getEligibilityTime();
}

static int comparePacketsByEligibilityTime(Packet *a, Packet *b)
{
    auto dt = getPacketEligibilityTime(a) - getPacketEligibilityTime(b);
    if (dt < 0)
        return -1;
    else if (dt > 0)
        return 1;
    else
        return 0;
}

Register_Packet_Comparator_Function(PacketEligibilityTimeComparator, comparePacketsByEligibilityTime);

} // namespace inet

