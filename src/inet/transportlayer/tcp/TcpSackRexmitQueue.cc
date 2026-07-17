//
// Copyright (C) 2009-2010 Thomas Reschka
// Copyright (C) 2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"

namespace inet {

namespace tcp {

TcpSackRexmitQueue::TcpSackRexmitQueue()
{
    conn = nullptr;
    begin = end = 0;
}

TcpSackRexmitQueue::~TcpSackRexmitQueue()
{
    while (!rexmitQueue.empty())
        rexmitQueue.pop_front();
}

void TcpSackRexmitQueue::init(uint32_t seqNum)
{
    begin = seqNum;
    end = seqNum;
}

std::string TcpSackRexmitQueue::str() const
{
    std::stringstream out;

    out << "[" << begin << ".." << end << ")";
    return out.str();
}

std::string TcpSackRexmitQueue::detailedInfo() const
{
    std::stringstream out;
    out << str() << endl;

    uint j = 1;

    for (const auto& elem : rexmitQueue) {
        out << j << ". region: [" << elem.beginSeqNum << ".." << elem.endSeqNum
            << ") \t sacked=" << elem.sacked << "\t rexmitted=" << elem.rexmitted
            << endl;
        j++;
    }
    return out.str();
}

void TcpSackRexmitQueue::discardUpTo(uint32_t seqNum)
{
    ASSERT(seqLE(begin, seqNum) && seqLE(seqNum, end));

    if (!rexmitQueue.empty()) {
        auto i = rexmitQueue.begin();

        while ((i != rexmitQueue.end()) && seqLE(i->endSeqNum, seqNum)) // discard/delete regions from rexmit queue, which have been acked
            i = rexmitQueue.erase(i);

        if (i != rexmitQueue.end()) {
            ASSERT(seqLE(i->beginSeqNum, seqNum) && seqLess(seqNum, i->endSeqNum));
            i->beginSeqNum = seqNum;
        }
    }

    begin = seqNum;

    // TESTING queue:
    ASSERT(checkQueue());
}

void TcpSackRexmitQueue::enqueueSentData(uint32_t fromSeqNum, uint32_t toSeqNum)
{
    ASSERT(seqLE(begin, fromSeqNum) && seqLE(fromSeqNum, end));

    bool found = false;
    Region region;

    EV_INFO << "rexmitQ: " << str() << " enqueueSentData [" << fromSeqNum << ".." << toSeqNum << ")\n";

    ASSERT(seqLess(fromSeqNum, toSeqNum));

    if (rexmitQueue.empty() || (end == fromSeqNum)) {
        region.beginSeqNum = fromSeqNum;
        region.endSeqNum = toSeqNum;
        region.sacked = false;
        region.rexmitted = false;
        region.firstSentTime = region.lastSentTime = simTime();
        region.transmitCount = 1;
        region.lost = false;
        rexmitQueue.push_back(region);
        found = true;
        fromSeqNum = toSeqNum;
    }
    else {
        auto i = rexmitQueue.begin();

        while (i != rexmitQueue.end() && seqLE(i->endSeqNum, fromSeqNum))
            i++;

        ASSERT(i != rexmitQueue.end());
        ASSERT(seqLE(i->beginSeqNum, fromSeqNum) && seqLess(fromSeqNum, i->endSeqNum));

        if (i->beginSeqNum != fromSeqNum) {
            // chunk item
            region = *i;
            region.endSeqNum = fromSeqNum;
            rexmitQueue.insert(i, region);
            i->beginSeqNum = fromSeqNum;
        }

        while (i != rexmitQueue.end() && seqLE(i->endSeqNum, toSeqNum)) {
            i->rexmitted = true;
            i->lastSentTime = simTime();
            i->transmitCount++;
            // Deliberately KEEP i->lost: a lost mark persists across the
            // retransmission until the data is cumulatively or selectively
            // acked (Linux keeps TCPCB_LOST alongside TCPCB_SACKED_RETRANS --
            // lost_out and retrans_out coexist in tcp_packets_in_flight()).
            // Clearing it here made setPipe() count a retransmitted lost head
            // under BOTH its rules ((a) not-lost and (b) retransmitted), one
            // segment high, which starved PRR's ssthresh-pipe cap right after
            // the fast retransmit and stalled recovery into the RTO.
            fromSeqNum = i->endSeqNum;
            found = true;
            i++;
        }

        if (fromSeqNum != toSeqNum) {
            bool beforeEnd = (i != rexmitQueue.end());

            ASSERT(i == rexmitQueue.end() || seqLess(i->beginSeqNum, toSeqNum));

            region.beginSeqNum = fromSeqNum;
            region.endSeqNum = toSeqNum;
            region.sacked = beforeEnd ? i->sacked : false;
            region.rexmitted = beforeEnd;
            region.firstSentTime = region.lastSentTime = simTime();
            region.transmitCount = 1;
            region.lost = false;
            rexmitQueue.insert(i, region);
            found = true;
            fromSeqNum = toSeqNum;

            if (beforeEnd)
                i->beginSeqNum = toSeqNum;
        }
    }

    ASSERT(fromSeqNum == toSeqNum);

    if (!found) {
        EV_DEBUG << "Not found enqueueSentData(" << fromSeqNum << ", " << toSeqNum << ")\nThe Queue is:\n" << detailedInfo();
    }

    ASSERT(found);

    begin = rexmitQueue.front().beginSeqNum;
    end = rexmitQueue.back().endSeqNum;

    // TESTING queue:
    ASSERT(checkQueue());

//    tcpEV << "rexmitQ: rexmitQLength=" << getQueueLength() << "\n";
}

bool TcpSackRexmitQueue::checkQueue() const
{
    uint32_t b = begin;
    bool f = true;

    for (const auto& elem : rexmitQueue) {
        f = f && (b == elem.beginSeqNum);
        f = f && seqLess(elem.beginSeqNum, elem.endSeqNum);
        b = elem.endSeqNum;
    }

    f = f && (b == end);

    if (!f) {
        EV_DEBUG << "Invalid Queue\nThe Queue is:\n" << detailedInfo();
    }

    return f;
}

uint32_t TcpSackRexmitQueue::setSackedBit(uint32_t fromSeqNum, uint32_t toSeqNum)
{
    // lowest sequence number this call NEWLY marked sacked, skipping regions
    // that were ever retransmitted (a SACK for a retransmission is ambiguous,
    // Linux's !TCPCB_RETRANS rule); 0 = nothing new. Regions are kept in
    // sequence order, so the first hit is the lowest. Consumed by the caller's
    // reordering detection (a new SACK below the prior FACK proves reordering).

    if (seqLess(fromSeqNum, begin))
        fromSeqNum = begin;

    ASSERT(seqLess(fromSeqNum, end));
    ASSERT(seqLess(begin, toSeqNum) && seqLE(toSeqNum, end));
    ASSERT(seqLess(fromSeqNum, toSeqNum));

    bool found = false;
    uint32_t newlySackedLow = 0;

    if (!rexmitQueue.empty()) {
        auto i = rexmitQueue.begin();

        while (i != rexmitQueue.end() && seqLE(i->endSeqNum, fromSeqNum))
            i++;

        ASSERT(i != rexmitQueue.end() && seqLE(i->beginSeqNum, fromSeqNum) && seqLess(fromSeqNum, i->endSeqNum));

        if (i->beginSeqNum != fromSeqNum) {
            Region region = *i;

            region.endSeqNum = fromSeqNum;
            rexmitQueue.insert(i, region);
            i->beginSeqNum = fromSeqNum;
        }

        while (i != rexmitQueue.end() && seqLE(i->endSeqNum, toSeqNum)) {
            if (seqGE(i->beginSeqNum, fromSeqNum)) { // Search region in queue!
                found = true;
                if (!i->sacked && !i->rexmitted && newlySackedLow == 0)
                    newlySackedLow = i->beginSeqNum;
                i->sacked = true; // set sacked bit
            }

            i++;
        }

        if (i != rexmitQueue.end() && seqLess(i->beginSeqNum, toSeqNum) && seqLess(toSeqNum, i->endSeqNum)) {
            Region region = *i;

            if (!i->sacked && !i->rexmitted && newlySackedLow == 0)
                newlySackedLow = i->beginSeqNum;
            region.endSeqNum = toSeqNum;
            region.sacked = true;
            rexmitQueue.insert(i, region);
            i->beginSeqNum = toSeqNum;
        }
    }

    if (!found)
        EV_DETAIL << "FAILED to set sacked bit for region: [" << fromSeqNum << ".." << toSeqNum << "). Not found in retransmission queue.\n";

    ASSERT(checkQueue());
    return newlySackedLow;
}

bool TcpSackRexmitQueue::getSackedBit(uint32_t seqNum) const
{
    ASSERT(seqLE(begin, seqNum) && seqLE(seqNum, end));

    RexmitQueue::const_iterator i = rexmitQueue.begin();

    if (end == seqNum)
        return false;

    while (i != rexmitQueue.end() && seqLE(i->endSeqNum, seqNum))
        i++;

    ASSERT((i != rexmitQueue.end()) && seqLE(i->beginSeqNum, seqNum) && seqLess(seqNum, i->endSeqNum));

    return i->sacked;
}

uint32_t TcpSackRexmitQueue::getHighestSackedSeqNum() const
{
    for (RexmitQueue::const_reverse_iterator i = rexmitQueue.rbegin(); i != rexmitQueue.rend(); i++) {
        if (i->sacked)
            return i->endSeqNum;
    }

    return begin;
}

uint32_t TcpSackRexmitQueue::getHighestRexmittedSeqNum() const
{
    for (RexmitQueue::const_reverse_iterator i = rexmitQueue.rbegin(); i != rexmitQueue.rend(); i++) {
        if (i->rexmitted)
            return i->endSeqNum;
    }

    return begin;
}

uint32_t TcpSackRexmitQueue::checkRexmitQueueForSackedOrRexmittedSegments(uint32_t fromSeqNum) const
{
    ASSERT(seqLE(begin, fromSeqNum) && seqLE(fromSeqNum, end));

    if (rexmitQueue.empty() || (end == fromSeqNum))
        return 0;

    RexmitQueue::const_iterator i = rexmitQueue.begin();
    uint32_t bytes = 0;

    while (i != rexmitQueue.end() && seqLE(i->endSeqNum, fromSeqNum))
        i++;

    while (i != rexmitQueue.end() && ((i->sacked || i->rexmitted))) {
        ASSERT(seqLE(i->beginSeqNum, fromSeqNum) && seqLess(fromSeqNum, i->endSeqNum));

        bytes += (i->endSeqNum - fromSeqNum);
        fromSeqNum = i->endSeqNum;
        i++;
    }

    return bytes;
}

void TcpSackRexmitQueue::resetSackedBit()
{
    for (auto& elem : rexmitQueue)
        elem.sacked = false; // reset sacked bit
}

void TcpSackRexmitQueue::resetRexmittedBit()
{
    for (auto& elem : rexmitQueue)
        elem.rexmitted = false; // reset rexmitted bit
}

uint32_t TcpSackRexmitQueue::getTotalAmountOfSackedBytes() const
{
    uint32_t bytes = 0;

    for (const auto& elem : rexmitQueue) {
        if (elem.sacked)
            bytes += (elem.endSeqNum - elem.beginSeqNum);
    }

    return bytes;
}

uint32_t TcpSackRexmitQueue::getAmountOfSackedBytes(uint32_t fromSeqNum) const
{
    ASSERT(seqLE(begin, fromSeqNum) && seqLE(fromSeqNum, end));

    uint32_t bytes = 0;
    RexmitQueue::const_reverse_iterator i = rexmitQueue.rbegin();

    for (; i != rexmitQueue.rend() && seqLE(fromSeqNum, i->beginSeqNum); i++) {
        if (i->sacked)
            bytes += (i->endSeqNum - i->beginSeqNum);
    }

    if (i != rexmitQueue.rend()
        && seqLess(i->beginSeqNum, fromSeqNum) && seqLess(fromSeqNum, i->endSeqNum) && i->sacked)
    {
        bytes += (i->endSeqNum - fromSeqNum);
    }

    return bytes;
}

uint32_t TcpSackRexmitQueue::getNumOfDiscontiguousSacks(uint32_t fromSeqNum) const
{
    ASSERT(seqLE(begin, fromSeqNum) && seqLE(fromSeqNum, end));

    if (rexmitQueue.empty() || (fromSeqNum == end))
        return 0;

    RexmitQueue::const_iterator i = rexmitQueue.begin();
    uint32_t counter = 0;

    while (i != rexmitQueue.end() && seqLE(i->endSeqNum, fromSeqNum)) // search for seqNum
        i++;

    // search for discontiguous sacked regions
    bool prevSacked = false;

    while (i != rexmitQueue.end()) {
        if (i->sacked && !prevSacked)
            counter++;

        prevSacked = i->sacked;
        i++;
    }

    return counter;
}

void TcpSackRexmitQueue::checkSackBlock(uint32_t fromSeqNum, uint32_t& length, bool& sacked, bool& rexmitted) const
{
    ASSERT(seqLE(begin, fromSeqNum) && seqLess(fromSeqNum, end));

    RexmitQueue::const_iterator i = rexmitQueue.begin();

    while (i != rexmitQueue.end() && seqLE(i->endSeqNum, fromSeqNum)) // search for seqNum
        i++;

    ASSERT(i != rexmitQueue.end());
    ASSERT(seqLE(i->beginSeqNum, fromSeqNum) && seqLess(fromSeqNum, i->endSeqNum));

    length = (i->endSeqNum - fromSeqNum);
    sacked = i->sacked;
    rexmitted = i->rexmitted;
}

const TcpSackRexmitQueue::Region& TcpSackRexmitQueue::getRegion(uint32_t seqNum) const
{
    ASSERT(seqLE(begin, seqNum) && seqLess(seqNum, end));

    RexmitQueue::const_iterator i = rexmitQueue.begin();

    while (i != rexmitQueue.end() && seqLE(i->endSeqNum, seqNum)) // search for seqNum
        i++;

    ASSERT(i != rexmitQueue.end());
    ASSERT(seqLE(i->beginSeqNum, seqNum) && seqLess(seqNum, i->endSeqNum));

    return *i;
}

void TcpSackRexmitQueue::markLost(uint32_t fromSeqNum, uint32_t toSeqNum)
{
    if (seqLess(fromSeqNum, begin))
        fromSeqNum = begin;

    if (seqLE(toSeqNum, fromSeqNum))
        return;

    ASSERT(seqLess(fromSeqNum, end));
    ASSERT(seqLE(toSeqNum, end));

    if (!rexmitQueue.empty()) {
        auto i = rexmitQueue.begin();

        while (i != rexmitQueue.end() && seqLE(i->endSeqNum, fromSeqNum))
            i++;

        ASSERT(i != rexmitQueue.end() && seqLE(i->beginSeqNum, fromSeqNum) && seqLess(fromSeqNum, i->endSeqNum));

        if (i->beginSeqNum != fromSeqNum) { // split off the tail so lost applies exactly from fromSeqNum
            Region region = *i;

            region.endSeqNum = fromSeqNum;
            rexmitQueue.insert(i, region);
            i->beginSeqNum = fromSeqNum;
        }

        while (i != rexmitQueue.end() && seqLE(i->endSeqNum, toSeqNum)) {
            if (seqGE(i->beginSeqNum, fromSeqNum))
                i->lost = true;

            i++;
        }

        if (i != rexmitQueue.end() && seqLess(i->beginSeqNum, toSeqNum) && seqLess(toSeqNum, i->endSeqNum)) {
            Region region = *i;

            region.endSeqNum = toSeqNum;
            region.lost = true;
            rexmitQueue.insert(i, region);
            i->beginSeqNum = toSeqNum;
        }
    }

    ASSERT(checkQueue());
}

uint32_t TcpSackRexmitQueue::getTotalAmountOfRexmittedUnsackedBytes() const
{
    // Linux retrans_out equivalent: bytes retransmitted and still outstanding
    // (not SACKed; cumulatively acked regions have left the queue entirely).
    uint32_t bytes = 0;

    for (const auto& elem : rexmitQueue) {
        if (elem.rexmitted && !elem.sacked)
            bytes += (elem.endSeqNum - elem.beginSeqNum);
    }

    return bytes;
}

uint32_t TcpSackRexmitQueue::getTotalAmountOfLostBytes() const
{
    uint32_t bytes = 0;

    for (const auto& elem : rexmitQueue) {
        if (elem.lost && !elem.sacked)
            bytes += (elem.endSeqNum - elem.beginSeqNum);
    }

    return bytes;
}

uint32_t TcpSackRexmitQueue::getRexmittedBytesInFlight() const
{
    uint32_t bytes = 0;

    for (const auto& elem : rexmitQueue) {
        if (elem.rexmitted && !elem.sacked)
            bytes += (elem.endSeqNum - elem.beginSeqNum);
    }

    return bytes;
}

} // namespace tcp

} // namespace inet

