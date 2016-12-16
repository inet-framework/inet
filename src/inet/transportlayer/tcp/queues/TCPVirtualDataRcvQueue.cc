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

#include "inet/transportlayer/tcp/queues/TCPVirtualDataRcvQueue.h"

namespace inet {

namespace tcp {

Register_Class(TCPVirtualDataRcvQueue);

TCPVirtualDataRcvQueue::TCPVirtualDataRcvQueue() :
    TCPReceiveQueue(),
    rcv_nxt(-1)
{
}

TCPVirtualDataRcvQueue::~TCPVirtualDataRcvQueue()
{
}

void TCPVirtualDataRcvQueue::init(uint32 startSeq)
{
    rcv_nxt = startSeq;

    reorderBuffer.clear();
    reorderBuffer.setExpectedOffset(startSeq);
}

std::string TCPVirtualDataRcvQueue::info() const
{
    std::string res;
    char buf[32];
    sprintf(buf, "rcv_nxt=%u", rcv_nxt);
    res = buf;

    for (int i = 0; i < reorderBuffer.getNumRegions(); i++) {
        sprintf(buf, " [%u..%u)", reorderBuffer.getRegionOffset(i), reorderBuffer.getRegionEndOffset(i));
        res += buf;
    }
}

uint32_t TCPVirtualDataRcvQueue::insertBytesFromSegment(Packet *packet, TcpHeader *tcpseg)
{
    int64_t tcpHeaderLength = tcpseg->getHeaderLength();
    int64_t tcpPayloadLength = packet->getByteLength() - tcpHeaderLength;
    const auto& payload = packet->peekDataAt(tcpHeaderLength, tcpPayloadLength);
    uint32_t seq = tcpseg->getSequenceNo();

#ifndef NDEBUG
    if (!reorderBuffer.empty()) {
        uint32_t ob = offsetToSeq(reorderBuffer.getRegionOffset(0));
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

    if (seqGE(rcv_nxt, offsetToSeq(reorderBuffer.getRegionOffset(0))))
        rcv_nxt = offsetToSeq(reorderBuffer.getRegionOffset(0) + reorderBuffer.getRegionLength(0));

    return rcv_nxt;
}

cPacket *TCPVirtualDataRcvQueue::extractBytesUpTo(uint32_t seq)
{
    ASSERT(seqLE(seq, rcv_nxt));

    if (reorderBuffer.empty())
        return nullptr;

    auto chunk = reorderBuffer.popData();
    ASSERT(reorderBuffer.getExpectedOffset() <= seqToOffset(seq));

    Packet *msg = nullptr;

    if (chunk) {
        Packet *msg = new Packet("data");
        msg->append(chunk);
        return msg;
    }

    return nullptr;
}

uint32 TCPVirtualDataRcvQueue::getAmountOfBufferedBytes()
{
    uint32 bytes = 0;

    for (int i = 0; i < reorderBuffer.getNumRegions(); i++)
        bytes += reorderBuffer.getRegionLength(i);

    return bytes;
}

uint32 TCPVirtualDataRcvQueue::getAmountOfFreeBytes(uint32 maxRcvBuffer)
{
    uint32 usedRcvBuffer = getAmountOfBufferedBytes();
    uint32 freeRcvBuffer = maxRcvBuffer - usedRcvBuffer;
    return (maxRcvBuffer > usedRcvBuffer) ? freeRcvBuffer : 0;
}

uint32 TCPVirtualDataRcvQueue::getQueueLength()
{
    return reorderBuffer.getNumRegions();
}

void TCPVirtualDataRcvQueue::getQueueStatus()
{
    EV_DEBUG << "receiveQLength=" << reorderBuffer.getNumRegions() << " " << info() << "\n";
}

uint32 TCPVirtualDataRcvQueue::getLE(uint32 fromSeqNum)
{
    int64_t fs = seqToOffset(fromSeqNum);

    for (int i = 0; i < reorderBuffer.getNumRegions(); i++) {
        if (reorderBuffer.getRegionOffset(i) <= fs && fs < reorderBuffer.getRegionEndOffset(i))
            return offsetToSeq(reorderBuffer.getRegionOffset(i));
    }

    return fromSeqNum;
}

uint32 TCPVirtualDataRcvQueue::getRE(uint32 toSeqNum)
{
    int64_t fs = seqToOffset(toSeqNum);

    for (int i = 0; i < reorderBuffer.getNumRegions(); i++) {
        if (reorderBuffer.getRegionOffset(i) < fs && fs <= reorderBuffer.getRegionEndOffset(i))
            return offsetToSeq(reorderBuffer.getRegionEndOffset(i));
    }

    return toSeqNum;
}

uint32 TCPVirtualDataRcvQueue::getFirstSeqNo()
{
    if (reorderBuffer.getNumRegions() == 0)
        return rcv_nxt;
    return seqMin(offsetToSeq(reorderBuffer.getRegionOffset(0)), rcv_nxt);
}

} // namespace tcp

} // namespace inet

