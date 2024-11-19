//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009 Thomas Reschka
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

void TcpReceiveQueue::init(uint32_t startSeq)
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

uint32_t TcpReceiveQueue::insertBytesFromSegment(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader)
{
    B tcpHeaderLength = tcpHeader->getHeaderLength();
    B tcpPayloadLength = tcpSegment->getDataLength() - tcpHeaderLength;
    uint32_t seq = tcpHeader->getSequenceNo();
    uint32_t offs = 0;
    uint32_t buffSeq = offsetToSeq(reorderBuffer.getExpectedOffset());

#ifndef NDEBUG
    if (!reorderBuffer.isEmpty()) {
        uint32_t ob = offsetToSeq(reorderBuffer.getRegionStartOffset(0));
        uint32_t oe = offsetToSeq(reorderBuffer.getRegionEndOffset(reorderBuffer.getNumRegions() - 1));
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
    const auto& payload = tcpSegment->peekDataAt(tcpHeaderLength + B(offs), tcpPayloadLength);
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

uint32_t TcpReceiveQueue::getAmountOfBufferedBytes()
{
    uint32_t bytes = 0;

    for (int i = 0; i < reorderBuffer.getNumRegions(); i++)
        bytes += reorderBuffer.getRegionLength(i).get<B>();

    return bytes;
}

uint32_t TcpReceiveQueue::getAmountOfFreeBytes(uint32_t maxRcvBuffer)
{
    uint32_t usedRcvBuffer = getAmountOfBufferedBytes();
    uint32_t freeRcvBuffer = maxRcvBuffer - usedRcvBuffer;
    return (maxRcvBuffer > usedRcvBuffer) ? freeRcvBuffer : 0;
}

uint32_t TcpReceiveQueue::getQueueLength()
{
    return reorderBuffer.getNumRegions();
}

void TcpReceiveQueue::getQueueStatus()
{
    EV_DEBUG << "receiveQLength=" << reorderBuffer.getNumRegions() << " " << str() << "\n";
}

uint32_t TcpReceiveQueue::getLE(uint32_t fromSeqNum)
{
    B fs = seqToOffset(fromSeqNum);

    for (int i = 0; i < reorderBuffer.getNumRegions(); i++) {
        if (reorderBuffer.getRegionStartOffset(i) <= fs && fs < reorderBuffer.getRegionEndOffset(i))
            return offsetToSeq(reorderBuffer.getRegionStartOffset(i));
    }

    return fromSeqNum;
}

uint32_t TcpReceiveQueue::getRE(uint32_t toSeqNum)
{
    B fs = seqToOffset(toSeqNum);

    for (int i = 0; i < reorderBuffer.getNumRegions(); i++) {
        if (reorderBuffer.getRegionStartOffset(i) < fs && fs <= reorderBuffer.getRegionEndOffset(i))
            return offsetToSeq(reorderBuffer.getRegionEndOffset(i));
    }

    return toSeqNum;
}

uint32_t TcpReceiveQueue::getFirstSeqNo()
{
    if (reorderBuffer.getNumRegions() == 0)
        return rcv_nxt;
    return seqMin(offsetToSeq(reorderBuffer.getRegionStartOffset(0)), rcv_nxt);
}

} // namespace tcp

} // namespace inet

