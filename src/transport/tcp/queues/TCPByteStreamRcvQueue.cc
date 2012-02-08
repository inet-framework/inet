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


#include "INETDefs.h"

#include "TCPByteStreamRcvQueue.h"

#include "ByteArrayMessage.h"
#include "TCPCommand_m.h"
#include "TCPSegment.h"

Register_Class(TCPByteStreamRcvQueue);


bool TCPByteStreamRcvQueue::Region::merge(const TCPVirtualDataRcvQueue::Region* _other)
{
    const Region *other = dynamic_cast<const Region *>(_other);

    if (!other)
        throw cRuntimeError("merge(): cannot cast (TCPVirtualDataRcvQueue::Region *) to type 'TCPDataStreamRcvQueue::Region *'");

    if (seqLess(end, other->begin) || seqLess(other->end, begin))
        return false;

    uint32 nbegin = seqMin(begin, other->begin);
    uint32 nend = seqMax(end, other->end);

    if (nbegin != begin || nend != end)
    {
        char *buff = new char[nend-nbegin];

        if (nbegin != begin)
            other->data.copyDataToBuffer(buff, begin - nbegin);

        data.copyDataToBuffer(buff + begin - nbegin, end - begin);

        if (nend != end)
            other->data.copyDataToBuffer(buff + end - nbegin, nend - end);

        begin = nbegin;
        end = nend;
        data.assignBuffer(buff, end - begin);
    }

    return true;
}

TCPByteStreamRcvQueue::Region* TCPByteStreamRcvQueue::Region::split(uint32 seq)
{
    ASSERT(seqGreater(seq, begin) && seqLess(seq, end));

    Region *reg = new Region(begin, seq);
    reg->data.setDataFromByteArray(data, 0, seq - begin);
    data.truncateData(seq - begin);
    begin = seq;
    return reg;
}

void TCPByteStreamRcvQueue::Region::copyTo(cPacket* msg_) const
{
    ASSERT(getLength() == data.getDataArraySize());

    ByteArrayMessage *msg = check_and_cast<ByteArrayMessage *>(msg_);
    TCPVirtualDataRcvQueue::Region::copyTo(msg);
    msg->setByteArray(data);
}

////////////////////////////////////////////////////////////////////

TCPByteStreamRcvQueue::~TCPByteStreamRcvQueue()
{
}

std::string TCPByteStreamRcvQueue::info() const
{
    std::stringstream os;

    os << "rcv_nxt=" << rcv_nxt;

    for (RegionList::const_iterator i=regionList.begin(); i!=regionList.end(); ++i)
    {
        os << " [" << (*i)->getBegin() << ".." << (*i)->getEnd() <<")";
    }

    os << " " << regionList.size() << "msgs";

    return os.str();
}

cPacket *TCPByteStreamRcvQueue::extractBytesUpTo(uint32 seq)
{
    cPacket *msg = NULL;
    TCPVirtualDataRcvQueue::Region *reg = extractTo(seq);
    if (reg)
    {
        msg = new ByteArrayMessage("data");
        reg->copyTo(msg);
        delete reg;
    }
    return msg;
}

TCPVirtualDataRcvQueue::Region* TCPByteStreamRcvQueue::createRegionFromSegment(TCPSegment *tcpseg)
{
    ASSERT(tcpseg->getPayloadLength() == tcpseg->getByteArray().getDataArraySize());

    Region *region = new Region(tcpseg->getSequenceNo(),
            tcpseg->getSequenceNo()+tcpseg->getPayloadLength(), tcpseg->getByteArray());

    return region;
}

