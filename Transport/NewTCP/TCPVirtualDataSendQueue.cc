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

void TCPVirtualDataSendQueue::enqueueAppData(cMessage *msg)
{
    end += msg->length() >> 3;
    delete msg;
}

uint32 TCPVirtualDataSendQueue::bufferEndSeq()
{
    return end;
}

TCPSegment *TCPVirtualDataSendQueue::createSegmentWithBytes(uint32 fromSeq, ulong numBytes)
{
    ASSERT(seqLE(begin,fromSeq) && seqLE(fromSeq+numBytes,end));

    TCPSegment *tcpseg = new TCPSegment("tcpseg");
    tcpseg->setSequenceNo(fromSeq);
    tcpseg->setPayloadLength(numBytes);
    return tcpseg;
}

void TCPVirtualDataSendQueue::discardUpTo(uint32 seqNum)
{
    ASSERT(seqLE(begin,seqNum) && seqLE(seqNum,end));
    begin = seqNum;
}

