//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include "TCPMsgBasedSendQueue.h"

Register_Class(TCPMsgBasedSendQueue);

TCPMsgBasedSendQueue::TCPMsgBasedSendQueue() : TCPSendQueue()
{
    begin = end = 0;
}

TCPMsgBasedSendQueue::~TCPMsgBasedSendQueue()
{
    for (PayloadQueue::iterator it=payloadQueue.begin(); it!=payloadQueue.end(); ++it)
        delete it->msg;
}

void TCPMsgBasedSendQueue::init(uint32 startSeq)
{
    begin = startSeq;
    end = startSeq;
}

std::string TCPMsgBasedSendQueue::info() const
{
    std::stringstream out;
    out << "[" << begin << ".." << end << "), " << payloadQueue.size() << " packets";
    return out.str();
}

void TCPMsgBasedSendQueue::enqueueAppData(cMessage *msg)
{
    //tcpEV << "sendQ: " << info() << " enqueueAppData(bytes=" << msg->byteLength() << ")\n";
    end += msg->byteLength();

    Payload payload;
    payload.endSequenceNo = end;
    payload.msg = msg;
    payloadQueue.push_back(payload);
}

uint32 TCPMsgBasedSendQueue::bufferEndSeq()
{
    return end;
}

TCPSegment *TCPMsgBasedSendQueue::createSegmentWithBytes(uint32 fromSeq, ulong numBytes)
{
    //tcpEV << "sendQ: " << info() << " createSeg(seq=" << fromSeq << " len=" << numBytes << ")\n";
    ASSERT(seqLE(begin,fromSeq) && seqLE(fromSeq+numBytes,end));

    TCPSegment *tcpseg = new TCPSegment();
    tcpseg->setSequenceNo(fromSeq);
    tcpseg->setPayloadLength(numBytes);

    // add payload messages whose endSequenceNo is between fromSeq and fromSeq+numBytes
    PayloadQueue::iterator i = payloadQueue.begin();
    while (i!=payloadQueue.end() && seqLE(i->endSequenceNo, fromSeq))
        ++i;
    uint32 toSeq = fromSeq+numBytes;
    const char *payloadName = NULL;
    while (i!=payloadQueue.end() && seqLE(i->endSequenceNo, toSeq))
    {
        if (!payloadName) payloadName = i->msg->name();
        tcpseg->addPayloadMessage((cMessage *)i->msg->dup(), i->endSequenceNo);
        ++i;
    }

    // give segment a name
    char msgname[80];
    if (!payloadName)
        sprintf(msgname, "tcpseg(l=%lu,%dmsg)", numBytes, tcpseg->payloadArraySize());
    else
        sprintf(msgname, "%.10s(l=%lu,%dmsg)", payloadName, numBytes, tcpseg->payloadArraySize());
    tcpseg->setName(msgname);

    return tcpseg;
}

void TCPMsgBasedSendQueue::discardUpTo(uint32 seqNum)
{
    //tcpEV << "sendQ: " << info() << " discardUpTo(seq=" << seqNum << ")\n";
    ASSERT(seqLE(begin,seqNum) && seqLE(seqNum,end));
    begin = seqNum;

    // remove payload messages whose endSequenceNo is below seqNum
    while (!payloadQueue.empty() && seqLE(payloadQueue.front().endSequenceNo, seqNum))
    {
        delete payloadQueue.front().msg;
        payloadQueue.pop_front();
    }
}

