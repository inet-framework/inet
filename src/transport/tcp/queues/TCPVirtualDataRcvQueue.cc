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

#include "TCPCommand.h"


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
    if (seqLess(end, other.begin))
        return BEFORE;
    if (end == other.begin)
        return BEFORE_TOUCH;
    if (begin == other.end)
        return AFTER_TOUCH;
    if (seqLess(other.end, begin))
        return AFTER;
    return OVERLAP;
}

void TCPVirtualDataRcvQueue::Region::copyTo(TCPDataMsg* msg) const
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

void TCPVirtualDataRcvQueue::setConnection(TCPConnection *_conn)
{
    ASSERT(_conn);

    TCPReceiveQueue::setConnection(_conn);
}

std::string TCPVirtualDataRcvQueue::info() const
{
    std::string res;
    char buf[32];
    sprintf(buf, "rcv_nxt=%u ", rcv_nxt);
    res = buf;

    for (RegionList::const_iterator i=regionList.begin(); i!=regionList.end(); ++i)
    {
        sprintf(buf, "[%u..%u) ", (*i)->getBegin(), (*i)->getEnd());
        res += buf;
    }
    return res;
}

uint32 TCPVirtualDataRcvQueue::insertBytesFromSegment(TCPSegment *tcpseg)
{
    Region *region = new Region(tcpseg->getSequenceNo(), tcpseg->getSequenceNo()+tcpseg->getPayloadLength());
    return insertBytesFromRegion(region);
}

uint32 TCPVirtualDataRcvQueue::insertBytesFromRegion(TCPVirtualDataRcvQueue::Region *region)
{
    merge(region);
    if (seqGE(rcv_nxt, regionList.front()->getBegin()))
        rcv_nxt = regionList.front()->getEnd();
    return rcv_nxt;
}

//void TCPVirtualDataRcvQueue::merge(uint32 segmentBegin, uint32 segmentEnd)
void TCPVirtualDataRcvQueue::merge(TCPVirtualDataRcvQueue::Region *seg)
{
    // Here we have to update our existing regions with the octet range
    // tcpseg represents. We either have to insert tcpseg as a separate region
    // somewhere, or (if it overlaps with an existing region) extend
    // existing regions; we also may have to merge existing regions if
    // they become overlapping (or touching) after adding tcpseg.

    RegionList::iterator i = regionList.begin();
    Region::CompareStatus cmp;
    while (i != regionList.end() && Region::AFTER != (cmp = (*i)->compare(*seg)))
    {
        RegionList::iterator old = i++;
        if (cmp != Region::BEFORE)
        {
            seg->merge(*old);
            delete *old;
            regionList.erase(old);
        }
    }
    regionList.insert(i, seg);
}

TCPDataMsg *TCPVirtualDataRcvQueue::extractBytesUpTo(uint32 seq, ulong maxBytes)
{
    TCPDataMsg *msg = NULL;
    Region *reg = extractTo(seq, maxBytes);
    if (reg)
    {
        msg = new TCPDataMsg("data");
        reg->copyTo(msg);
    }
    delete reg;
    return msg;
}

TCPVirtualDataRcvQueue::Region* TCPVirtualDataRcvQueue::extractTo(uint32 seq, ulong maxBytes)
{
    ASSERT(seqLE(seq,rcv_nxt));

    if(regionList.empty())
        return NULL;

    Region * reg = regionList.front();
    if (seqLE(seq, reg->getBegin()))
        return NULL;

    if (seqGE(seq, reg->getEnd()))
    {
        regionList.pop_front();
        return reg;
    }
    return reg->split(seq);
}

ulong TCPVirtualDataRcvQueue::getExtractableBytesUpTo(uint32 seq)
{
    ASSERT(seqLE(seq,rcv_nxt));

    if(regionList.empty())
        return 0;

    Region *reg = regionList.front();

    // full 1st region
    return (reg->getLengthTo(seq));
}

uint32 TCPVirtualDataRcvQueue::getAmountOfBufferedBytes()
{
    uint32 bytes=0;

    for (RegionList::iterator i = regionList.begin(); i != regionList.end(); i++)
    {
        bytes += (*i)->getLength();
    }
    return bytes;
}

uint32 TCPVirtualDataRcvQueue::getAmountOfFreeBytes(uint32 maxRcvBuffer)
{
    uint32 usedRcvBuffer = getAmountOfBufferedBytes();
    uint32 freeRcvBuffer = maxRcvBuffer - usedRcvBuffer;
    return freeRcvBuffer;
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
    while (i!=regionList.end())
    {
        if (seqLE((*i)->getBegin(), fromSeqNum) && seqLE(fromSeqNum, (*i)->getEnd()))
        {
//            tcpEV << "Enqueued region: [" << i->begin << ".." << i->end << ")\n";
            if (seqLess((*i)->getBegin(), fromSeqNum))
                return (*i)->getBegin();
            return fromSeqNum;
        }
        i++;
    }
    return fromSeqNum;
}

uint32 TCPVirtualDataRcvQueue::getRE(uint32 toSeqNum)
{
    RegionList::iterator i = regionList.begin();
    while (i!=regionList.end())
    {
        if (seqLE((*i)->getBegin(), toSeqNum) && seqLE(toSeqNum, (*i)->getEnd()))
        {
//            tcpEV << "Enqueued region: [" << i->begin << ".." << i->end << ")\n";
            if (seqLess(toSeqNum, (*i)->getEnd()))
                return (*i)->getEnd();
            return toSeqNum;
        }
        i++;
    }
    return toSeqNum;
}
