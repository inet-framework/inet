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

#include "inet/queueing/function/PacketComparatorFunction.h"

namespace inet {
namespace queueing {

static int comparePacketCreationTime(Packet *packet1, Packet *packet2)
{
    auto d = (packet2->getCreationTime() - packet1->getCreationTime()).raw();
    if (d == 0)
        return 0;
    else if (d > 0)
        return 1;
    else
        return -1;
}

Register_Packet_Comparator_Function(PacketCreationTimeComparator, comparePacketCreationTime);

static int comparePacketByData(Packet *packet1, Packet *packet2)
{
    const auto& data1 = packet1->peekDataAt<BytesChunk>(B(0), B(1));
    auto byte1 = data1->getBytes().at(0);
    const auto& data2 = packet2->peekDataAt<BytesChunk>(B(0), B(1));
    auto byte2 = data2->getBytes().at(0);
    return byte2 - byte1;
}

Register_Packet_Comparator_Function(PacketDataComparator, comparePacketByData);

} // namespace queueing
} // namespace inet

