//
// Copyright (C) 2004 Andras Varga
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

Register_Class(TCPMsgBasedRcvQueue);


TCPMsgBasedRcvQueue::TCPMsgBasedRcvQueue() : TCPVirtualDataRcvQueue()
{
}

TCPMsgBasedRcvQueue::~TCPMsgBasedRcvQueue()
{
}

void TCPMsgBasedRcvQueue::init(uint32 startSeq)
{
    TCPVirtualDataRcvQueue::init(startSeq);
}

std::string TCPMsgBasedRcvQueue::info() const
{
    std::string res;
    char buf[32];
    sprintf(buf, "rcv_nxt=%u ", rcv_nxt);
    res = buf;

    for (RegionList::const_iterator i=regionList.begin(); i!=regionList.end(); ++i)
    {
        sprintf(buf, "[%u..%u) ", i->begin, i->end);
        res+=buf;
    }
    sprintf(buf, "%u msgs", payloadList.size());
    res+=buf;

    return res;
}

uint32 TCPMsgBasedRcvQueue::insertBytesFromSegment(TCPSegment *tcpseg)
{
    TCPVirtualDataRcvQueue::insertBytesFromSegment(tcpseg);

    cPacket *msg;
    uint32 endSeqNo;
    while ((msg=tcpseg->removeFirstPayloadMessage(endSeqNo))!=NULL)
    {
        // insert, avoiding duplicates
        PayloadList::iterator i = payloadList.find(endSeqNo);
        if (i!=payloadList.end()) {delete msg; continue;}
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

