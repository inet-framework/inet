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
                " Queue: offset=" << payloadList.begin()->first <<
                ", length=" << payloadList.begin()->second->getByteLength() << endl;
        delete payloadList.begin()->second;
        payloadList.erase(payloadList.begin());
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
    while (NULL != (msg = tcpseg->removeFirstPayloadMessage(endSeqNo)))
    {
        // insert, avoiding duplicates
        PayloadList::iterator i = payloadList.find(endSeqNo);
        if (i != payloadList.end())
            delete msg;
        else
            payloadList[endSeqNo] = msg;
    }

    return rcv_nxt;
}

cPacket *TCPMsgBasedRcvQueue::extractBytesUpTo(uint32 seq)
{
    extractTo(seq);

    // pass up payload messages, in sequence number order
    if (payloadList.empty() || seqGreater(payloadList.begin()->first, seq))
        return NULL;

    cPacket *msg = payloadList.begin()->second;
    payloadList.erase(payloadList.begin());
    return msg;
}
