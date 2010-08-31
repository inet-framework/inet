//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2010 Zoltan Bojthe
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


#include <omnetpp.h>

#include "TCPDataStreamRcvQueue.h"

#include "TCPCommand.h"
#include "TCPSegmentWithData.h"

Register_Class(TCPDataStreamRcvQueue);


bool TCPDataStreamRcvQueue::Region::merge(const TCPVirtualDataRcvQueue::Region* _other)
{
    const Region *other = dynamic_cast<const Region *>(_other);
    if (!other)
        throw cRuntimeError("check_and_cast(): cannot cast (TCPVirtualDataRcvQueue::Region *) to type 'TCPDataStreamRcvQueue::Region'");

    if (seqLess(end, other->begin) || seqLess(other->end, begin))
        return false;

    uint32 nbegin = (seqLess(other->begin, begin)) ? other->begin : begin;
    uint32 nend = (seqLess(end, other->end)) ? other->end : end;
    if (nbegin != begin || nend != end)
    {
        char * buff = new char[nend-nbegin];
        if (nbegin != begin)
            other->data.copyDataToBuffer(buff, begin - nbegin);
        data.copyDataToBuffer(buff + begin - nbegin, end - begin);
        if (nend != end)
            other->data.copyDataToBuffer(buff + end - nbegin, nend - end);
        begin = nbegin;
        end = nend;
        data.setDataFromBuffer(buff, end - begin);
        delete buff;
    }
    return true;
}

TCPDataStreamRcvQueue::Region* TCPDataStreamRcvQueue::Region::split(uint32 seq)
{
    ASSERT(seqGreater(seq, begin) && seqLess(seq, end));

    Region * reg = new Region(begin, seq);
    reg->data.setDataFromByteArray(data, 0, seq - begin);
    data.truncateData(seq - begin);
    begin = seq;
    return reg;
}

void TCPDataStreamRcvQueue::Region::copyTo(TCPDataMsg* msg) const
{
    ASSERT(getLength() == data.getDataArraySize());
    TCPVirtualDataRcvQueue::Region::copyTo(msg);
    msg->setByteArray(data);
}

////////////////////////////////////////////////////////////////////

TCPDataStreamRcvQueue::~TCPDataStreamRcvQueue()
{
}

std::string TCPDataStreamRcvQueue::info() const
{
    std::string res;
    char buf[32];
    sprintf(buf, "rcv_nxt=%u ", rcv_nxt);
    res = buf;

    for (RegionList::const_iterator i=regionList.begin(); i!=regionList.end(); ++i)
    {
        sprintf(buf, "[%u..%u) ", (*i)->getBegin(), (*i)->getEnd());
        res+=buf;
    }
    sprintf(buf, "%u msgs", regionList.size());
    res+=buf;

    return res;
}

uint32 TCPDataStreamRcvQueue::insertBytesFromSegment(TCPSegment *tcpseg_)
{
    TCPSegmentWithBytes *tcpseg = check_and_cast<TCPSegmentWithBytes *>(tcpseg_);
    Region *region = new Region(tcpseg->getSequenceNo(),
            tcpseg->getSequenceNo()+tcpseg->getPayloadLength(), tcpseg->getByteArray());
    return insertBytesFromRegion(region);
}

