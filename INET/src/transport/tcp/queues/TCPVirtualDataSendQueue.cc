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


#include "TCPVirtualDataSendQueue.h"

Register_Class(TCPVirtualDataSendQueue);

TCPVirtualDataSendQueue::TCPVirtualDataSendQueue() : TCPSendQueue()
{
    begin = end = 0;
}

TCPVirtualDataSendQueue::~TCPVirtualDataSendQueue()
{
}

void TCPVirtualDataSendQueue::init(uint32 startSeq)
{
    begin = startSeq;
    end = startSeq;
}

std::string TCPVirtualDataSendQueue::info() const
{
    std::stringstream out;
    out << "[" << begin << ".." << end << ")";
    return out.str();
}

void TCPVirtualDataSendQueue::enqueueAppData(cMessage *msg)
{
    //tcpEV << "sendQ: " << info() << " enqueueAppData(bytes=" << msg->byteLength() << ")\n";
    end += msg->byteLength();
    delete msg;
}

uint32 TCPVirtualDataSendQueue::bufferEndSeq()
{
    return end;
}

TCPSegment *TCPVirtualDataSendQueue::createSegmentWithBytes(uint32 fromSeq, ulong numBytes)
{
    //tcpEV << "sendQ: " << info() << " createSeg(seq=" << fromSeq << " len=" << numBytes << ")\n";
    ASSERT(seqLE(begin,fromSeq) && seqLE(fromSeq+numBytes,end));

    char msgname[32];
    sprintf(msgname, "tcpseg(l=%lu)", numBytes);

    TCPSegment *tcpseg = new TCPSegment(msgname);
    tcpseg->setSequenceNo(fromSeq);
    tcpseg->setPayloadLength(numBytes);
    return tcpseg;
}

void TCPVirtualDataSendQueue::discardUpTo(uint32 seqNum)
{
    //tcpEV << "sendQ: " << info() << " discardUpTo(seq=" << seqNum << ")\n";
    ASSERT(seqLE(begin,seqNum) && seqLE(seqNum,end));
    begin = seqNum;
}

