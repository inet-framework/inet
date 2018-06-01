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
    //tcpEV << "sendQ: " << str() << " enqueueAppData(bytes=" << msg->getByteLength() << ")\n";
    dataBuffer.push(msg->peekDataAt(B(0), msg->getDataLength()));
    end += msg->getByteLength();
    if (seqLess(end, begin))
        throw cRuntimeError("Send queue is full");
    delete msg;
}

uint32 TcpSendQueue::getBufferStartSeq()
{
    return begin;
}

uint32 TcpSendQueue::getBufferEndSeq()
{
    return end;
}

Packet *TcpSendQueue::createSegmentWithBytes(uint32 fromSeq, ulong numBytes)
{
    //tcpEV << "sendQ: " << str() << " createSeg(seq=" << fromSeq << " len=" << numBytes << ")\n";

    ASSERT(seqLE(begin, fromSeq) && seqLE(fromSeq + numBytes, end));

    char msgname[32];
    sprintf(msgname, "tcpseg(l=%lu)", numBytes);

    Packet *packet = new Packet(msgname);
    const auto& payload = dataBuffer.peekAt(B(fromSeq - begin), B(numBytes));   //get data from buffer
    //std::cout << "#: " << getSimulation()->getEventNumber() << ", T: " << simTime() << ", SENDER: " << conn->getTcpMain()->getParentModule()->getFullName() << ", DATA: " << payload << std::endl;
    packet->insertAtBack(payload);
    return packet;
}

void TcpSendQueue::discardUpTo(uint32 seqNum)
{
    //tcpEV << "sendQ: " << str() << " discardUpTo(seq=" << seqNum << ")\n";

    ASSERT(seqLE(begin, seqNum) && seqLE(seqNum, end));

    if (seqNum != begin) {
        dataBuffer.pop(B(seqNum - begin));
        begin = seqNum;
    }
}

} // namespace tcp

} // namespace inet

