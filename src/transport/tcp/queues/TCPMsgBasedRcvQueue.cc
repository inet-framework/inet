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


#include "TCPMsgBasedRcvQueue.h"

#include "TCPCommand_m.h"
#include "TCPSegment.h"

Register_Class(TCPMsgBasedRcvQueue);


TCPMsgBasedRcvQueue::TCPMsgBasedRcvQueue() : TCPVirtualDataRcvQueue()
{
}

TCPMsgBasedRcvQueue::~TCPMsgBasedRcvQueue()
{
    while (! payloadList.empty())
    {
        EV << "SendQueue Destructor: Drop msg from " << this->getFullPath() <<
                " Queue: offset=" << payloadList.front().seqNo <<
                ", length=" << payloadList.front().packet->getByteLength() << endl;
        delete payloadList.front().packet;
        payloadList.pop_front();
    }
}

void TCPMsgBasedRcvQueue::init(uint32 startSeq)
{
    TCPVirtualDataRcvQueue::init(startSeq);
}

std::string TCPMsgBasedRcvQueue::info() const
{
    std::stringstream os;

    os << "rcv_nxt=" << rcv_nxt;

    for (RegionList::const_iterator i = regionList.begin(); i != regionList.end(); ++i)
    {
        os << " [" << (*i)->getBegin() << ".." << (*i)->getEnd() << ")";
    }

    os << " " << payloadList.size() << " msgs";

    return os.str();
}

uint32 TCPMsgBasedRcvQueue::insertBytesFromSegment(TCPSegment *tcpseg)
{
    TCPVirtualDataRcvQueue::insertBytesFromSegment(tcpseg);

    cPacket *msg;
    uint32 endSeqNo;
    PayloadList::iterator i = payloadList.begin();
    while (NULL != (msg = tcpseg->removeFirstPayloadMessage(endSeqNo)))
    {
        while (i != payloadList.end() && seqLess(i->seqNo, endSeqNo))
            ++i;

        // insert, avoiding duplicates
        if (i != payloadList.end() && i->seqNo == endSeqNo)
            delete msg;
        else
        {
            i = payloadList.insert(i,PayloadItem(endSeqNo, msg));
            ASSERT(seqLE(payloadList.front().seqNo, payloadList.back().seqNo));
        }
    }

    return rcv_nxt;
}

cPacket *TCPMsgBasedRcvQueue::extractBytesUpTo(uint32 seq)
{
    cPacket *msg = NULL;
    if (!payloadList.empty() && seqLess(payloadList.begin()->seqNo, seq))
        seq = payloadList.begin()->seqNo;

    Region *reg = extractTo(seq);
    if (reg)
    {
        if (!payloadList.empty() && payloadList.begin()->seqNo == reg->getEnd())
        {
            msg = payloadList.begin()->packet;
            payloadList.erase(payloadList.begin());
        }
        delete reg;
    }
    return msg;
}
