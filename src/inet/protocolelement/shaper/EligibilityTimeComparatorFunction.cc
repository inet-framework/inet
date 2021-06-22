//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/protocolelement/shaper/EligibilityTimeTag_m.h"
#include "inet/queueing/function/PacketComparatorFunction.h"

namespace inet {

static simtime_t getPacketEligibilityTime(cObject *object)
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

