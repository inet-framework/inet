//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/tcp/flavours/TcpSegmentTransmitInfoList.h"

#include <algorithm> // min,max

namespace inet {

namespace tcp {

void TcpSegmentTransmitInfoList::set(uint32_t beg, uint32_t end, simtime_t sentTime)
{
    ASSERT(seqLess(beg, end));
    ASSERT(regions.empty() || (seqLE(regions.front().beg, beg) && seqLE(beg, regions.back().end)));

    if (regions.empty() || (regions.back().end == beg))
        regions.push_back(Item(beg, end, sentTime, sentTime, 1));
    else if (end == regions.front().beg)
        regions.push_front(Item(beg, end, sentTime, sentTime, 1));
    else {
        auto i = regions.begin();
        while (seqLE(i->end, beg))
            ++i;

        ASSERT(seqLE(i->beg, beg));
        if (beg != i->beg) {
            regions.insert(i, Item(i->beg, beg, i->firstSentTime, i->lastSentTime, i->transmitCount));
            i->beg = beg;
        }
        while (i != regions.end() && seqLE(i->end, end)) {
            ASSERT(beg == i->beg);
            if (i->firstSentTime > sentTime)
                i->firstSentTime = sentTime;
            if (i->lastSentTime < sentTime)
                i->lastSentTime = sentTime;
            i->transmitCount++;
            beg = i->end;
            ++i;
        }

        if (beg != end) {
            if (i != regions.end()) {
                ASSERT(beg == i->beg);
                simtime_t firstSent = std::min(i->firstSentTime, sentTime);
                simtime_t lastSent = std::max(i->lastSentTime, sentTime);
//                if (firstSent > sentTime)
//                    firstSent = sentTime;
//                if (lastSent < sentTime)
//                    lastSent = sentTime;
                regions.insert(i, Item(beg, end, firstSent, lastSent, i->transmitCount + 1));
                i->beg = end;
            }
            else
                regions.push_back(Item(beg, end, sentTime, sentTime, 1));
        }
    }
}

const TcpSegmentTransmitInfoList::Item *TcpSegmentTransmitInfoList::get(uint32_t seq) const
{
    for (const auto& elem : regions) {
        if (seqLess(seq, elem.beg))
            break;
        if (seqLE(elem.beg, seq) && seqLess(seq, elem.end))
            return &(elem);
    }
    return nullptr;
}

void TcpSegmentTransmitInfoList::clearTo(uint32_t endseq)
{
    while (!regions.empty() && seqLE(regions.front().end, endseq))
        regions.pop_front();
    if (!regions.empty() && seqLess(regions.front().beg, endseq))
        regions.front().beg = endseq;
}

} // namespace tcp

} // namespace inet

