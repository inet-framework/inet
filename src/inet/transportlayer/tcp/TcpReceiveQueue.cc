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

#include "inet/transportlayer/tcp/TcpReceiveQueue.h"

namespace inet {

namespace tcp {

Register_Class(TcpReceiveQueue);

TcpReceiveQueue::TcpReceiveQueue() :
    rcv_nxt(-1)
{
}

TcpReceiveQueue::~TcpReceiveQueue()
{
}

void TcpReceiveQueue::init(uint32 startSeq)
{
    rcv_nxt = startSeq;

    reorderBuffer.clear();
    reorderBuffer.setExpectedOffset(B(startSeq));
}

std::string TcpReceiveQueue::str() const
{
    std::ostringstream buf;
    buf << "rcv_nxt=" << rcv_nxt;

    for (int i = 0; i < reorderBuffer.getNumRegions(); i++) {
        buf << " [" << offsetToSeq(reorderBuffer.getRegionStartOffset(i)) << ".." << offsetToSeq(reorderBuffer.getRegionEndOffset(i)) << ")";
    }
    return buf.str();
}

uint32_t TcpReceiveQueue::insertBytesFromSegment(Packet *packet, const Ptr<const TcpHeader>& tcpseg)
{
    B tcpHeaderLength = tcpseg->getHeaderLength();
    B tcpPayloadLength = packet->getDataLength() - tcpHeaderLength;
    uint32_t seq = tcpseg->getSequenceNo();
    uint32_t offs = 0;
    uint32_t buffSeq = offsetToSeq(reorderBuffer.getExpectedOffset());

#ifndef NDEBUG
    if (!reorderBuffer.isEmpty()) {
        uint32_t ob = offsetToSeq(reorderBuffer.getRegionStartOffset(0));
        uint32_t oe = offsetToSeq(reorderBuffer.getRegionEndOffset(reorderBuffer.getNumRegions()-1));
        uint32_t nb = seq;
        uint32_t ne = seq + tcpPayloadLength.get();
        uint32_t minb = seqMin(ob, nb);
        uint32_t maxe = seqMax(oe, ne);
        if (seqGE(minb, oe) || seqGE(minb, ne) || seqGE(ob, maxe) || seqGE(nb, maxe))
            throw cRuntimeError("The new segment is [%u, %u) out of the acceptable range at the queue %s",
                    nb, ne, str().c_str());
    }
#endif // ifndef NDEBUG

    if (seqLess(seq, buffSeq)) {
        offs = buffSeq - seq;
        seq = buffSeq;
        tcpPayloadLength -= B(offs);
    }
    const auto& payload = packet->peekDataAt(tcpHeaderLength + B(offs), tcpPayloadLength);
    reorderBuffer.replace(seqToOffset(seq), payload);

    if (seqGE(rcv_nxt, offsetToSeq(reorderBuffer.getRegionStartOffset(0))))
        rcv_nxt = offsetToSeq(reorderBuffer.getRegionStartOffset(0) + reorderBuffer.getRegionLength(0));

    return rcv_nxt;
}

Packet *TcpReceiveQueue::extractBytesUpTo(uint32_t seq)
{
    ASSERT(seqLE(seq, rcv_nxt));

    if (reorderBuffer.isEmpty())
        return nullptr;

    auto seqOffs = seqToOffset(seq);
    auto maxLength = seqOffs - reorderBuffer.getExpectedOffset();
    if (maxLength <= b(0))
        return nullptr;
    auto chunk = reorderBuffer.popAvailableData(maxLength);
    ASSERT(reorderBuffer.getExpectedOffset() <= seqToOffset(seq));

    if (chunk) {
        Packet *msg = new Packet("data");
        msg->insertAtBack(chunk);
        return msg;
    }

    return nullptr;
}

uint32 TcpReceiveQueue::getAmountOfBufferedBytes()
{
    uint32 bytes = 0;

    for (int i = 0; i < reorderBuffer.getNumRegions(); i++)
        bytes += B(reorderBuffer.getRegionLength(i)).get();

    return bytes;
}

uint32 TcpReceiveQueue::getAmountOfFreeBytes(uint32 maxRcvBuffer)
{
    uint32 usedRcvBuffer = getAmountOfBufferedBytes();
    uint32 freeRcvBuffer = maxRcvBuffer - usedRcvBuffer;
    return (maxRcvBuffer > usedRcvBuffer) ? freeRcvBuffer : 0;
}

uint32 TcpReceiveQueue::getQueueLength()
{
    return reorderBuffer.getNumRegions();
}

void TcpReceiveQueue::getQueueStatus()
{
    EV_DEBUG << "receiveQLength=" << reorderBuffer.getNumRegions() << " " << str() << "\n";
}

uint32 TcpReceiveQueue::getLE(uint32 fromSeqNum)
{
    B fs = seqToOffset(fromSeqNum);

    for (int i = 0; i < reorderBuffer.getNumRegions(); i++) {
        if (reorderBuffer.getRegionStartOffset(i) <= fs && fs < reorderBuffer.getRegionEndOffset(i))
            return offsetToSeq(reorderBuffer.getRegionStartOffset(i));
    }

    return fromSeqNum;
}

uint32 TcpReceiveQueue::getRE(uint32 toSeqNum)
{
    B fs = seqToOffset(toSeqNum);

    for (int i = 0; i < reorderBuffer.getNumRegions(); i++) {
        if (reorderBuffer.getRegionStartOffset(i) < fs && fs <= reorderBuffer.getRegionEndOffset(i))
            return offsetToSeq(reorderBuffer.getRegionEndOffset(i));
    }

    return toSeqNum;
}

uint32 TcpReceiveQueue::getFirstSeqNo()
{
    if (reorderBuffer.getNumRegions() == 0)
        return rcv_nxt;
    return seqMin(offsetToSeq(reorderBuffer.getRegionStartOffset(0)), rcv_nxt);
}

} // namespace tcp

} // namespace inet

