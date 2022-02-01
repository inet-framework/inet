//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

