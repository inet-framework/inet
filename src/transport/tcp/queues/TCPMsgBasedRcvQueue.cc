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

#include "TCPCommand.h"
#include "TCPSegmentWithData.h"

Register_Class(TCPMsgBasedRcvQueue);


TCPMsgBasedRcvQueue::TCPMsgBasedRcvQueue() : TCPVirtualDataRcvQueue()
{
    extractedBytes = extractedPayloadBytes = 0;
    isPayloadExtractAtFirst = false;
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

void TCPMsgBasedRcvQueue::setConnection(TCPConnection *_conn)
{
    ASSERT(_conn);

    TCPReceiveQueue::setConnection(_conn);
    isPayloadExtractAtFirst = conn->isSendingObjectUpAtFirstByteEnabled();
}


void TCPMsgBasedRcvQueue::init(uint32 startSeq)
{
    TCPVirtualDataRcvQueue::init(startSeq);
    extractedBytes = extractedPayloadBytes = 0;
    isPayloadExtractAtFirst = false;
}

std::string TCPMsgBasedRcvQueue::info() const
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
    sprintf(buf, "%u msgs", payloadList.size());
    res+=buf;

    return res;
}

uint32 TCPMsgBasedRcvQueue::insertBytesFromSegment(TCPSegment *tcpseg_)
{
    TCPSegmentWithMessages *tcpseg = check_and_cast<TCPSegmentWithMessages *>(tcpseg_);

    TCPVirtualDataRcvQueue::insertBytesFromSegment(tcpseg);

    cPacket *msg;
    uint64 streamOffs, segmentOffs;
    while (NULL != (msg = tcpseg->removeFirstPayloadMessage(streamOffs, segmentOffs)))
    {
        // insert, avoiding duplicates
        PayloadList::iterator i = payloadList.find(streamOffs);
        if (i != payloadList.end())
            delete msg;
        else
            payloadList[streamOffs] = msg;
    }

    return rcv_nxt;
}

TCPDataMsg* TCPMsgBasedRcvQueue::extractBytesUpTo(uint32 seq, ulong maxBytes)
{
    TCPDataMsg *msg = NULL;
    cPacket *objMsg = NULL;
    uint64 nextPayloadBegin = extractedPayloadBytes;
    uint64 nextPayloadLength = 0;
    uint64 nextPayloadOffs = 0;

    if (!payloadList.empty())
    {
        nextPayloadBegin = payloadList.begin()->first;
        nextPayloadLength = payloadList.begin()->second->getByteLength();
    }
    uint64 nextPayloadEnd = nextPayloadBegin + nextPayloadLength;

    ASSERT(nextPayloadBegin == extractedPayloadBytes);

    if (isPayloadExtractAtFirst)
    {
        nextPayloadOffs = nextPayloadBegin;
        ASSERT(extractedBytes <= extractedPayloadBytes);
        ASSERT(nextPayloadBegin >= extractedBytes);
        if (nextPayloadBegin == extractedBytes)
        {
            if (maxBytes > nextPayloadLength)
                maxBytes = nextPayloadLength;
        }
        else // nextPayloadBegin > extractedBytes
        {
            ulong extractableBytes = nextPayloadBegin - extractedBytes;
            if (maxBytes > extractableBytes)
                maxBytes = extractableBytes;
        }
    }
    else
    {
        nextPayloadOffs = nextPayloadEnd -1;
        ulong extractableBytes = nextPayloadEnd - extractedBytes;
        ASSERT(extractedBytes >= extractedPayloadBytes);
        if (maxBytes > extractableBytes)
            maxBytes = extractableBytes;
    }

    Region *reg = extractTo(seq, maxBytes);
    if (reg)
    {
        ulong bytes = reg->getLength();
        delete reg;

        if (bytes)
        {
            msg = new TCPDataMsg("DATA");
            msg->setByteLength(bytes);
            if (!payloadList.empty() && extractedBytes <= nextPayloadOffs && nextPayloadOffs < extractedBytes + bytes)
            {
                objMsg = payloadList.begin()->second;
                payloadList.erase(payloadList.begin());
                msg->setPayloadPacket(objMsg);
                msg->setIsPayloadStart(isPayloadExtractAtFirst);
                extractedPayloadBytes = nextPayloadEnd;
            }
            msg->setIsPayloadStart(isPayloadExtractAtFirst);
            extractedBytes += bytes;
        }
    }
    return msg;
}

