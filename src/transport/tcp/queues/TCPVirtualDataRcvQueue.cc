//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2009 Thomas Reschka
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


#include "TCPVirtualDataRcvQueue.h"


Register_Class(TCPVirtualDataRcvQueue);

bool TCPVirtualDataRcvQueue::Region::merge(const TCPVirtualDataRcvQueue::Region* other)
{
    if (seqLess(end, other->begin) || seqLess(other->end, begin))
        return false;
    if (seqLess(other->begin, begin))
        begin = other->begin;
    if (seqLess(end, other->end))
        end = other->end;
    return true;
}

TCPVirtualDataRcvQueue::Region* TCPVirtualDataRcvQueue::Region::split(uint32 seq)
{
    ASSERT(seqGreater(seq, begin) && seqLess(seq, end));

    Region * reg = new Region(begin, seq);
    begin = seq;
    return reg;
}

TCPVirtualDataRcvQueue::Region::CompareStatus TCPVirtualDataRcvQueue::Region::compare(const TCPVirtualDataRcvQueue::Region& other) const
{
    if (end == other.begin)
        return BEFORE_TOUCH;
    if (begin == other.end)
        return AFTER_TOUCH;
    if (seqLess(end, other.begin))
        return BEFORE;
    if (seqLess(other.end, begin))
        return AFTER;
    return OVERLAP;
}

void TCPVirtualDataRcvQueue::Region::copyTo(cPacket *msg) const
{
    msg->setByteLength(getLength());
}

ulong TCPVirtualDataRcvQueue::Region::getLengthTo(uint32 seq) const
{
    // seq below 1st region
    if (seqLE(seq, begin))
        return 0;

    if (seqLess(seq, end)) // part of 1st region
        return (ulong)(seq - begin);

    return (ulong)(end - begin);
}

////////////////////////////////////////////////////////////////////

TCPVirtualDataRcvQueue::TCPVirtualDataRcvQueue() : TCPReceiveQueue()
{
}

TCPVirtualDataRcvQueue::~TCPVirtualDataRcvQueue()
{
    while (!regionList.empty())
    {
        delete regionList.front();
        regionList.pop_front();
    }
}

void TCPVirtualDataRcvQueue::init(uint32 startSeq)
{
    rcv_nxt = startSeq;

    while (!regionList.empty())
    {
        delete regionList.front();
        regionList.pop_front();
    }
}

std::string TCPVirtualDataRcvQueue::info() const
{
    std::string res;
    char buf[32];
    sprintf(buf, "rcv_nxt=%u", rcv_nxt);
    res = buf;

    for (RegionList::const_iterator i=regionList.begin(); i!=regionList.end(); ++i)
    {
        sprintf(buf, " [%u..%u)", (*i)->getBegin(), (*i)->getEnd());
        res += buf;
    }
    return res;
}

TCPVirtualDataRcvQueue::Region* TCPVirtualDataRcvQueue::createRegionFromSegment(TCPSegment *tcpseg)
{
    Region *region = new Region(tcpseg->getSequenceNo(), tcpseg->getSequenceNo()+tcpseg->getPayloadLength());
    return region;
}
uint32 TCPVirtualDataRcvQueue::insertBytesFromSegment(TCPSegment *tcpseg)
{
    Region *region = createRegionFromSegment(tcpseg);

#ifndef NDEBUG
    if (!regionList.empty())
    {
        uint32 ob = regionList.front()->getBegin();
        uint32 oe = regionList.back()->getEnd();
        uint32 nb = region->getBegin();
        uint32 ne = region->getEnd();
        uint32 minb = seqMin(ob, nb);
        uint32 maxe = seqMax(oe, ne);
        if (seqGE(minb, oe) || seqGE(minb, ne) || seqGE(ob, maxe) || seqGE(nb, maxe))
            throw cRuntimeError("The new segment is [%u, %u) out of the acceptable range at the queue %s",
                    region->getBegin(), region->getEnd(), info().c_str());
    }
#endif

    merge(region);

    if (seqGE(rcv_nxt, regionList.front()->getBegin()))
        rcv_nxt = regionList.front()->getEnd();

    return rcv_nxt;
}

void TCPVirtualDataRcvQueue::merge(TCPVirtualDataRcvQueue::Region *seg)
{
    // Here we have to update our existing regions with the octet range
    // tcpseg represents. We either have to insert tcpseg as a separate region
    // somewhere, or (if it overlaps with an existing region) extend
    // existing regions; we also may have to merge existing regions if
    // they become overlapping (or touching) after adding tcpseg.

    RegionList::reverse_iterator i = regionList.rbegin();
    Region::CompareStatus cmp;

    while (i != regionList.rend() && Region::BEFORE != (cmp = (*i)->compare(*seg)))
    {
        if (cmp != Region::AFTER)
        {
            if (seg->merge(*i))
            {
                delete *i;
                i = (RegionList::reverse_iterator)(regionList.erase((++i).base()));
                continue;
            }
            else
                throw cRuntimeError("Model error: merge of region [%u,%u) with [%u,%u) unsuccessful", (*i)->getBegin(), (*i)->getEnd(), seg->getBegin(), seg->getEnd());
        }
        ++i;
    }

    regionList.insert(i.base(), seg);
}

cPacket *TCPVirtualDataRcvQueue::extractBytesUpTo(uint32 seq)
{
    cPacket *msg = NULL;
    Region *reg = extractTo(seq);

    if (reg)
    {
        msg = new cPacket("data");
        reg->copyTo(msg);
        delete reg;
    }
    return msg;
}

TCPVirtualDataRcvQueue::Region* TCPVirtualDataRcvQueue::extractTo(uint32 seq)
{
    ASSERT(seqLE(seq, rcv_nxt));

    if (regionList.empty())
        return NULL;

    Region *reg = regionList.front();
    uint32 beg = reg->getBegin();

    if (seqLE(seq, beg))
        return NULL;

    if (seqGE(seq, reg->getEnd()))
    {
        regionList.pop_front();
        return reg;
    }

    return reg->split(seq);
}

uint32 TCPVirtualDataRcvQueue::getAmountOfBufferedBytes()
{
    uint32 bytes = 0;

    for (RegionList::iterator i = regionList.begin(); i != regionList.end(); i++)
        bytes += (*i)->getLength();

    return bytes;
}

uint32 TCPVirtualDataRcvQueue::getAmountOfFreeBytes(uint32 maxRcvBuffer)
{
    uint32 usedRcvBuffer = getAmountOfBufferedBytes();
    uint32 freeRcvBuffer = maxRcvBuffer - usedRcvBuffer;
    return (maxRcvBuffer > usedRcvBuffer) ? freeRcvBuffer : 0;
}

uint32 TCPVirtualDataRcvQueue::getQueueLength()
{
    return regionList.size();
}

void TCPVirtualDataRcvQueue::getQueueStatus()
{
    tcpEV << "receiveQLength=" << regionList.size() << " " << info() << "\n";
}


uint32 TCPVirtualDataRcvQueue::getLE(uint32 fromSeqNum)
{
    RegionList::iterator i = regionList.begin();

    while (i != regionList.end())
    {
        if (seqLE((*i)->getBegin(), fromSeqNum) && seqLess(fromSeqNum, (*i)->getEnd()))
        {
//            tcpEV << "Enqueued region: [" << i->begin << ".." << i->end << ")\n";
            return (*i)->getBegin();
        }

        i++;
    }

    return fromSeqNum;
}

uint32 TCPVirtualDataRcvQueue::getRE(uint32 toSeqNum)
{
    RegionList::iterator i = regionList.begin();

    while (i != regionList.end())
    {
        if (seqLess((*i)->getBegin(), toSeqNum) && seqLE(toSeqNum, (*i)->getEnd()))
        {
//            tcpEV << "Enqueued region: [" << i->begin << ".." << i->end << ")\n";
            return (*i)->getEnd();
        }

        i++;
    }

    return toSeqNum;
}
