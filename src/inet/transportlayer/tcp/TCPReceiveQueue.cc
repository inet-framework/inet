//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2009 Thomas Reschka
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

#include "inet/transportlayer/tcp/TCPReceiveQueue.h"

namespace inet {

namespace tcp {

Register_Class(TCPReceiveQueue);

TCPReceiveQueue::TCPReceiveQueue() :
    rcv_nxt(-1)
{
}

TCPReceiveQueue::~TCPReceiveQueue()
{
}

void TCPReceiveQueue::init(uint32 startSeq)
{
    rcv_nxt = startSeq;

    reorderBuffer.clear();
    reorderBuffer.setExpectedOffset(startSeq);
}

std::string TCPReceiveQueue::str() const
{
    std::string res;
    char buf[32];
    sprintf(buf, "rcv_nxt=%u", rcv_nxt);
    res = buf;

    for (int i = 0; i < reorderBuffer.getNumRegions(); i++) {
        sprintf(buf, " [%lu..%lu)", reorderBuffer.getRegionStartOffset(i), reorderBuffer.getRegionEndOffset(i));
        res += buf;
    }
    return res;
}

uint32_t TCPReceiveQueue::insertBytesFromSegment(Packet *packet, TcpHeader *tcpseg)
{
    int64_t tcpHeaderLength = tcpseg->getHeaderLength();
    int64_t tcpPayloadLength = packet->getByteLength() - tcpHeaderLength;
    uint32_t seq = tcpseg->getSequenceNo();
    uint32_t offs = 0;
    uint32_t buffSeq = offsetToSeq(reorderBuffer.getExpectedOffset());
    if (seqLess(seq, buffSeq)) {
        offs = buffSeq - seq;
        seq = buffSeq;
        tcpPayloadLength -= offs;
    }
    const auto& payload = packet->peekDataAt(tcpHeaderLength + offs, tcpPayloadLength);

#ifndef NDEBUG
    if (!reorderBuffer.isEmpty()) {
        uint32_t ob = offsetToSeq(reorderBuffer.getRegionStartOffset(0));
        uint32_t oe = offsetToSeq(reorderBuffer.getRegionEndOffset(reorderBuffer.getNumRegions()-1));
        uint32_t nb = seq;
        uint32_t ne = seq + tcpPayloadLength;
        uint32_t minb = seqMin(ob, nb);
        uint32_t maxe = seqMax(oe, ne);
        if (seqGE(minb, oe) || seqGE(minb, ne) || seqGE(ob, maxe) || seqGE(nb, maxe))
            throw cRuntimeError("The new segment is [%u, %u) out of the acceptable range at the queue %s",
                    nb, ne, info().c_str());
    }
#endif // ifndef NDEBUG
    reorderBuffer.replace(seqToOffset(seq), payload);

    if (seqGE(rcv_nxt, offsetToSeq(reorderBuffer.getRegionStartOffset(0))))
        rcv_nxt = offsetToSeq(reorderBuffer.getRegionStartOffset(0) + reorderBuffer.getRegionLength(0));

    return rcv_nxt;
}

cPacket *TCPReceiveQueue::extractBytesUpTo(uint32_t seq)
{
    ASSERT(seqLE(seq, rcv_nxt));

    if (reorderBuffer.isEmpty())
        return nullptr;

    auto chunk = reorderBuffer.popData();
    ASSERT(reorderBuffer.getExpectedOffset() <= seqToOffset(seq));

    Packet *msg = nullptr;

    if (chunk) {
        // TODO: KLUDGE: to match fingerprints with packet containing objects
        kludgeQueue.push(chunk);
        auto data = kludgeQueue.peekAt(0, kludgeQueue.getBufferLength());
        if (data->getChunkType() == Chunk::TYPE_SLICE)
            return nullptr;
        else if (data->getChunkType() == Chunk::TYPE_SEQUENCE) {
            auto sequenceChunk = std::static_pointer_cast<SequenceChunk>(data);
            for (auto chunk : sequenceChunk->getChunks())
                if (chunk->getChunkType() == Chunk::TYPE_SLICE)
                    return nullptr;
        }
        else
            kludgeQueue.clear();
        chunk = data;
        // TODO: KLDUGE: end

        //std::cout << "#: " << getSimulation()->getEventNumber() << ", T: " << simTime() << ", RECEIVER: " << conn->getTcpMain()->getParentModule()->getFullName() << ", DATA: " << chunk << std::endl;
        Packet *msg = new Packet("data");
        msg->append(chunk);
        return msg;
    }

    return nullptr;
}

uint32 TCPReceiveQueue::getAmountOfBufferedBytes()
{
    uint32 bytes = 0;

    for (int i = 0; i < reorderBuffer.getNumRegions(); i++)
        bytes += reorderBuffer.getRegionLength(i);

    return bytes;
}

uint32 TCPReceiveQueue::getAmountOfFreeBytes(uint32 maxRcvBuffer)
{
    uint32 usedRcvBuffer = getAmountOfBufferedBytes();
    uint32 freeRcvBuffer = maxRcvBuffer - usedRcvBuffer;
    return (maxRcvBuffer > usedRcvBuffer) ? freeRcvBuffer : 0;
}

uint32 TCPReceiveQueue::getQueueLength()
{
    return reorderBuffer.getNumRegions();
}

void TCPReceiveQueue::getQueueStatus()
{
    EV_DEBUG << "receiveQLength=" << reorderBuffer.getNumRegions() << " " << info() << "\n";
}

uint32 TCPReceiveQueue::getLE(uint32 fromSeqNum)
{
    int64_t fs = seqToOffset(fromSeqNum);

    for (int i = 0; i < reorderBuffer.getNumRegions(); i++) {
        if (reorderBuffer.getRegionStartOffset(i) <= fs && fs < reorderBuffer.getRegionEndOffset(i))
            return offsetToSeq(reorderBuffer.getRegionStartOffset(i));
    }

    return fromSeqNum;
}

uint32 TCPReceiveQueue::getRE(uint32 toSeqNum)
{
    int64_t fs = seqToOffset(toSeqNum);

    for (int i = 0; i < reorderBuffer.getNumRegions(); i++) {
        if (reorderBuffer.getRegionStartOffset(i) < fs && fs <= reorderBuffer.getRegionEndOffset(i))
            return offsetToSeq(reorderBuffer.getRegionEndOffset(i));
    }

    return toSeqNum;
}

uint32 TCPReceiveQueue::getFirstSeqNo()
{
    if (reorderBuffer.getNumRegions() == 0)
        return rcv_nxt;
    return seqMin(offsetToSeq(reorderBuffer.getRegionStartOffset(0)), rcv_nxt);
}

} // namespace tcp

} // namespace inet

