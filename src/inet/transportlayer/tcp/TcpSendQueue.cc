//
// Copyright (C) 2004 OpenSim Ltd.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/transportlayer/tcp/TcpSendQueue.h"

namespace inet {

namespace tcp {

Register_Class(TcpSendQueue);

TcpSendQueue::TcpSendQueue()
{
}

TcpSendQueue::~TcpSendQueue()
{
}

void TcpSendQueue::init(uint32 startSeq)
{
    begin = startSeq;
    end = startSeq;
    dataBuffer.clear();          // clear dataBuffer
}

std::string TcpSendQueue::str() const
{
    std::stringstream out;
    out << "[" << begin << ".." << end << ")" << dataBuffer;
    return out.str();
}

void TcpSendQueue::enqueueAppData(Packet *msg)
{
    dataBuffer.push(msg->peekDataAt(B(0), msg->getDataLength()));
    end += msg->getByteLength();
    if (seqLess(end, begin))
        throw cRuntimeError("Send queue is full");
    delete msg;
}

uint32 TcpSendQueue::getBufferStartSeq() const
{
    return begin;
}

uint32 TcpSendQueue::getBufferEndSeq() const
{
    return end;
}

uint32 TcpSendQueue::getBytesAvailable(uint32 fromSeq) const
{
    return seqLess(fromSeq, end) ? end - fromSeq : 0;
}

Packet *TcpSendQueue::createSegmentWithBytes(uint32 fromSeq, uint32 numBytes)
{
    ASSERT(seqLE(begin, fromSeq) && seqLE(fromSeq + numBytes, end));

    char msgname[32];
    sprintf(msgname, "tcpseg(l=%u)", (unsigned int)numBytes);

    Packet *tcpSegment = new Packet(msgname);
    const auto& payload = dataBuffer.peekAt(B(fromSeq - begin), B(numBytes));   //get data from buffer
    tcpSegment->insertAtBack(payload);
    return tcpSegment;
}

void TcpSendQueue::discardUpTo(uint32 seqNum)
{
    ASSERT(seqLE(begin, seqNum) && seqLE(seqNum, end));

    if (seqNum != begin) {
        dataBuffer.pop(B(seqNum - begin));
        begin = seqNum;
    }
}

} // namespace tcp

} // namespace inet

