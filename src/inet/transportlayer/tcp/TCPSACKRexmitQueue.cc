//
// Copyright (C) 2009-2010 Thomas Reschka
// Copyright (C) 2011 Zoltan Bojthe
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/transportlayer/tcp/TCPSACKRexmitQueue.h"

namespace inet {

namespace tcp {

TCPSACKRexmitQueue::TCPSACKRexmitQueue()
{
    conn = NULL;
    begin = end = 0;
}

TCPSACKRexmitQueue::~TCPSACKRexmitQueue()
{
    while (!rexmitQueue.empty())
        rexmitQueue.pop_front();
}

void TCPSACKRexmitQueue::init(uint32 seqNum)
{
    begin = seqNum;
    end = seqNum;
}

std::string TCPSACKRexmitQueue::str() const
{
    std::stringstream out;

    out << "[" << begin << ".." << end << ")";
    return out.str();
}

void TCPSACKRexmitQueue::info() const
{
    EV_DETAIL << str() << endl;

    uint j = 1;

    for (RexmitQueue::const_iterator i = rexmitQueue.begin(); i != rexmitQueue.end(); i++) {
        EV_DETAIL << j << ". region: [" << i->beginSeqNum << ".." << i->endSeqNum
                  << ") \t sacked=" << i->sacked << "\t rexmitted=" << i->rexmitted
                  << endl;
        j++;
    }
}

void TCPSACKRexmitQueue::discardUpTo(uint32 seqNum)
{
    ASSERT(seqLE(begin, seqNum) && seqLE(seqNum, end));

    if (!rexmitQueue.empty()) {
        RexmitQueue::iterator i = rexmitQueue.begin();

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

void TCPSACKRexmitQueue::enqueueSentData(uint32 fromSeqNum, uint32 toSeqNum)
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
        rexmitQueue.push_back(region);
        found = true;
        fromSeqNum = toSeqNum;
    }
    else {
        RexmitQueue::iterator i = rexmitQueue.begin();

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
            rexmitQueue.insert(i, region);
            found = true;
            fromSeqNum = toSeqNum;

            if (beforeEnd)
                i->beginSeqNum = toSeqNum;
        }
    }

    ASSERT(fromSeqNum == toSeqNum);

    if (!found) {
        EV_DEBUG << "Not found enqueueSentData(" << fromSeqNum << ", " << toSeqNum << ")\nThe Queue is:\n";
        info();
    }

    ASSERT(found);

    begin = rexmitQueue.front().beginSeqNum;
    end = rexmitQueue.back().endSeqNum;

    // TESTING queue:
    ASSERT(checkQueue());

    // tcpEV << "rexmitQ: rexmitQLength=" << getQueueLength() << "\n";
}

bool TCPSACKRexmitQueue::checkQueue() const
{
    uint32 b = begin;
    bool f = true;

    for (RexmitQueue::const_iterator i = rexmitQueue.begin(); i != rexmitQueue.end(); i++) {
        f = f && (b == i->beginSeqNum);
        f = f && seqLess(i->beginSeqNum, i->endSeqNum);
        b = i->endSeqNum;
    }

    f = f && (b == end);

    if (!f) {
        EV_DEBUG << "Invalid Queue\nThe Queue is:\n";
        info();
    }

    return f;
}

void TCPSACKRexmitQueue::setSackedBit(uint32 fromSeqNum, uint32 toSeqNum)
{
    if (seqLess(fromSeqNum, begin))
        fromSeqNum = begin;

    ASSERT(seqLess(fromSeqNum, end));
    ASSERT(seqLess(begin, toSeqNum) && seqLE(toSeqNum, end));
    ASSERT(seqLess(fromSeqNum, toSeqNum));

    bool found = false;

    if (!rexmitQueue.empty()) {
        RexmitQueue::iterator i = rexmitQueue.begin();

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
            if (seqGE(i->beginSeqNum, fromSeqNum)) {    // Search region in queue!
                found = true;
                i->sacked = true;    // set sacked bit
            }

            i++;
        }

        if (i != rexmitQueue.end() && seqLess(i->beginSeqNum, toSeqNum) && seqLess(toSeqNum, i->endSeqNum)) {
            Region region = *i;

            region.endSeqNum = toSeqNum;
            region.sacked = true;
            rexmitQueue.insert(i, region);
            i->beginSeqNum = toSeqNum;
        }
    }

    if (!found)
        EV_DETAIL << "FAILED to set sacked bit for region: [" << fromSeqNum << ".." << toSeqNum << "). Not found in retransmission queue.\n";

    ASSERT(checkQueue());
}

bool TCPSACKRexmitQueue::getSackedBit(uint32 seqNum) const
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

uint32 TCPSACKRexmitQueue::getHighestSackedSeqNum() const
{
    for (RexmitQueue::const_reverse_iterator i = rexmitQueue.rbegin(); i != rexmitQueue.rend(); i++) {
        if (i->sacked)
            return i->endSeqNum;
    }

    return begin;
}

uint32 TCPSACKRexmitQueue::getHighestRexmittedSeqNum() const
{
    for (RexmitQueue::const_reverse_iterator i = rexmitQueue.rbegin(); i != rexmitQueue.rend(); i++) {
        if (i->rexmitted)
            return i->endSeqNum;
    }

    return begin;
}

uint32 TCPSACKRexmitQueue::checkRexmitQueueForSackedOrRexmittedSegments(uint32 fromSeqNum) const
{
    ASSERT(seqLE(begin, fromSeqNum) && seqLE(fromSeqNum, end));

    if (rexmitQueue.empty() || (end == fromSeqNum))
        return 0;

    RexmitQueue::const_iterator i = rexmitQueue.begin();
    uint32 bytes = 0;

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

void TCPSACKRexmitQueue::resetSackedBit()
{
    for (RexmitQueue::iterator i = rexmitQueue.begin(); i != rexmitQueue.end(); i++)
        i->sacked = false; // reset sacked bit
}

void TCPSACKRexmitQueue::resetRexmittedBit()
{
    for (RexmitQueue::iterator i = rexmitQueue.begin(); i != rexmitQueue.end(); i++)
        i->rexmitted = false; // reset rexmitted bit
}

uint32 TCPSACKRexmitQueue::getTotalAmountOfSackedBytes() const
{
    uint32 bytes = 0;

    for (RexmitQueue::const_iterator i = rexmitQueue.begin(); i != rexmitQueue.end(); i++) {
        if (i->sacked)
            bytes += (i->endSeqNum - i->beginSeqNum);
    }

    return bytes;
}

uint32 TCPSACKRexmitQueue::getAmountOfSackedBytes(uint32 fromSeqNum) const
{
    ASSERT(seqLE(begin, fromSeqNum) && seqLE(fromSeqNum, end));

    uint32 bytes = 0;
    RexmitQueue::const_reverse_iterator i = rexmitQueue.rbegin();

    for ( ; i != rexmitQueue.rend() && seqLE(fromSeqNum, i->beginSeqNum); i++) {
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

uint32 TCPSACKRexmitQueue::getNumOfDiscontiguousSacks(uint32 fromSeqNum) const
{
    ASSERT(seqLE(begin, fromSeqNum) && seqLE(fromSeqNum, end));

    if (rexmitQueue.empty() || (fromSeqNum == end))
        return 0;

    RexmitQueue::const_iterator i = rexmitQueue.begin();
    uint32 counter = 0;

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

void TCPSACKRexmitQueue::checkSackBlock(uint32 fromSeqNum, uint32& length, bool& sacked, bool& rexmitted) const
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

} // namespace tcp

} // namespace inet

