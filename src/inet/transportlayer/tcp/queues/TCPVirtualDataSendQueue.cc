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

#include "inet/transportlayer/tcp/queues/TCPVirtualDataSendQueue.h"

namespace inet {

namespace tcp {

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
    dataBuffer.clear();          // clear dataBuffer
}

std::string TCPVirtualDataSendQueue::info() const
{
    std::stringstream out;
    out << "[" << begin << ".." << end << ")";
    return out.str();
}

void TCPVirtualDataSendQueue::enqueueAppData(Packet *msg)
{
    //tcpEV << "sendQ: " << info() << " enqueueAppData(bytes=" << msg->getByteLength() << ")\n";
    dataBuffer.push(msg->peekDataAt(0, msg->getByteLength()));
    end += msg->getByteLength();
    if (seqLess(end, begin))
        throw cRuntimeError("Send queue is full");
    delete msg;
}

uint32 TCPVirtualDataSendQueue::getBufferStartSeq()
{
    return begin;
}

uint32 TCPVirtualDataSendQueue::getBufferEndSeq()
{
    return end;
}

Packet *TCPVirtualDataSendQueue::createSegmentWithBytes(uint32 fromSeq, ulong numBytes)
{
    //tcpEV << "sendQ: " << info() << " createSeg(seq=" << fromSeq << " len=" << numBytes << ")\n";

    ASSERT(seqLE(begin, fromSeq) && seqLE(fromSeq + numBytes, end));

    char msgname[32];
    sprintf(msgname, "tcpseg(l=%lu)", numBytes);

    Packet *packet = new Packet(msgname);
    const auto& tcpseg = std::make_shared<TcpHeader>();
    tcpseg->setSequenceNo(fromSeq);
    const auto& payload = dataBuffer.peekAt(fromSeq-begin, numBytes);   //get data from buffer
    packet->pushHeader(tcpseg);
    packet->pushTrailer(payload);
    return packet;
}

void TCPVirtualDataSendQueue::discardUpTo(uint32 seqNum)
{
    //tcpEV << "sendQ: " << info() << " discardUpTo(seq=" << seqNum << ")\n";

    ASSERT(seqLE(begin, seqNum) && seqLE(seqNum, end));

    if (seqNum != begin) {
        dataBuffer.pop(seqNum - begin);
        begin = seqNum;
    }
}

} // namespace tcp

} // namespace inet

