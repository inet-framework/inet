//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TIMETAG_H
#define __INET_TIMETAG_H

#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/Packet.h"

namespace inet {

template<typename T>
void increaseTimeTag(const Ptr<Chunk>& chunk, simtime_t bitDuration, simtime_t packetDuration)
{
    chunk->mapAllTagsForUpdate<T>(b(0), chunk->getChunkLength(), [&] (b offset, b length, T *timeTag) {
        for (int i = 0; i < (int)timeTag->getBitTotalTimesArraySize(); i++)
            timeTag->setBitTotalTimes(i, timeTag->getBitTotalTimes(i) + bitDuration);
        for (int i = 0; i < (int)timeTag->getPacketTotalTimesArraySize(); i++)
            timeTag->setPacketTotalTimes(i, timeTag->getPacketTotalTimes(i) + packetDuration);
    });
}

template<typename T>
void increaseTimeTag(Packet *packet, simtime_t bitDuration, simtime_t packetDuration)
{
    packet->mapAllRegionTagsForUpdate<T>(b(0), packet->getTotalLength(), [&] (b offset, b length, const Ptr<T>& timeTag) {
        for (int i = 0; i < (int)timeTag->getBitTotalTimesArraySize(); i++)
            timeTag->setBitTotalTimes(i, timeTag->getBitTotalTimes(i) + bitDuration);
        for (int i = 0; i < (int)timeTag->getPacketTotalTimesArraySize(); i++)
            timeTag->setPacketTotalTimes(i, timeTag->getPacketTotalTimes(i) + packetDuration);
    });
}

} // namespace inet

#endif

