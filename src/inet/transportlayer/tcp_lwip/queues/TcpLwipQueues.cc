//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/tcp_lwip/queues/TcpLwipQueues.h"

#include <algorithm>

#include "inet/common/packet/serializer/BytesChunkSerializer.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/tcp_common/TcpHeaderSerializer.h"
#include "inet/transportlayer/tcp_lwip/TcpLwipConnection.h"

namespace inet {
namespace tcp {

Register_Class(TcpLwipSendQueue);

Register_Class(TcpLwipReceiveQueue);

// Assembles the region-tagged data for [seqNo, seqNo+length) from a seqno-keyed
// cache of segment payloads, slicing and concatenating as needed. Returns nullptr
// if any part of the range is not covered by the cache.
static const Ptr<const Chunk> assembleTaggedData(const std::map<uint32_t, Ptr<const Chunk>>& cache,
        uint32_t seqNo, unsigned int length)
{
    ChunkQueue assembled;
    uint32_t pos = seqNo;
    unsigned int remaining = length;

    while (remaining > 0) {
        auto it = cache.upper_bound(pos); // first entry starting after pos
        if (it == cache.begin())
            return nullptr; // no entry starts at or before pos
        --it; // entry with the greatest start <= pos
        uint32_t segSeq = it->first;
        const auto& segChunk = it->second;
        unsigned int segLen = segChunk->getChunkLength().get<B>();
        if ((int32_t)(pos - (segSeq + segLen)) >= 0)
            return nullptr; // pos is past this entry -> gap
        unsigned int sliceOffset = pos - segSeq;
        unsigned int sliceLen = std::min(remaining, segLen - sliceOffset);
        assembled.push(segChunk->peek(Chunk::Iterator(true, B(sliceOffset), -1), B(sliceLen)));
        pos += sliceLen;
        remaining -= sliceLen;
    }
    return assembled.pop(B(length));
}

// Drops cache entries whose data is entirely before upToSeq (delivered/acked).
static void pruneTaggedData(std::map<uint32_t, Ptr<const Chunk>>& cache, uint32_t upToSeq)
{
    for (auto it = cache.begin(); it != cache.end();) {
        uint32_t segEnd = it->first + it->second->getChunkLength().get<B>();
        if ((int32_t)(segEnd - upToSeq) <= 0)
            it = cache.erase(it);
        else
            ++it;
    }
}

TcpLwipSendQueue::TcpLwipSendQueue()
{
}

TcpLwipSendQueue::~TcpLwipSendQueue()
{
}

void TcpLwipSendQueue::setConnection(TcpLwipConnection *connP)
{
    dataBuffer.clear();
    taggedSegments.clear();
    connM = connP;
}

void TcpLwipSendQueue::enqueueAppData(Packet *msg)
{
    ASSERT(msg);
    const auto& data = msg->peekDataAt(B(0), B(msg->getByteLength()));
    // Cache the region tags before dataBuffer is drained into the (tag-less) lwIP
    // stack. This chunk will be buffered by lwIP right after the not-yet-sent data
    // already in dataBuffer, i.e. at sequence number snd_lbb + dataBuffer length.
    if (connM != nullptr && connM->pcbM != nullptr && data->getChunkLength() > b(0)) {
        uint32_t seq = connM->pcbM->snd_lbb + dataBuffer.getLength().get<B>();
        taggedSegments[seq] = data;
    }
    dataBuffer.push(data);
    delete msg;
}

unsigned int TcpLwipSendQueue::getBytesForTcpLayer(void *bufferP, unsigned int bufferLengthP) const
{
    ASSERT(bufferP);

    unsigned int length = dataBuffer.getLength().get<B>();
    if (bufferLengthP < length)
        length = bufferLengthP;
    if (length == 0)
        return 0;

    const auto& bytesChunk = dataBuffer.peek<BytesChunk>(B(length));
    return bytesChunk->copyToBuffer(static_cast<uint8_t *>(bufferP), length);
}

void TcpLwipSendQueue::dequeueTcpLayerMsg(unsigned int msgLengthP)
{
    dataBuffer.pop(B(msgLengthP));
}

unsigned long TcpLwipSendQueue::getBytesAvailable() const
{
    return dataBuffer.getLength().get<B>();
}

Packet *TcpLwipSendQueue::createSegmentWithBytes(const void *tcpDataP, unsigned int tcpLengthP)
{
    ASSERT(tcpDataP);

    const auto& bytes = makeShared<BytesChunk>((const uint8_t *)tcpDataP, tcpLengthP);
    auto packet = new Packet(nullptr, bytes);
    auto tcpHdr = packet->removeAtFront<TcpHeader>();
    int64_t numBytes = packet->getByteLength();

    // The lwIP stack works on raw bytes and has stripped the application data's
    // region tags. Restore them by replacing this segment's payload with the
    // corresponding (still region-tagged) chunk held in the send buffer, so the
    // tags travel to the peer. The send buffer's front byte has sequence number
    // SND.UNA (== pcb->lastack), which maps the segment to a buffer offset.
    if (numBytes > 0 && connM != nullptr && connM->pcbM != nullptr) {
        uint32_t seq = tcpHdr->getSequenceNo();
        const auto& appData = assembleTaggedData(taggedSegments, seq, numBytes);
        if (appData != nullptr) {
            packet->eraseAll();
            packet->insertAtBack(appData);
        }
        pruneTaggedData(taggedSegments, connM->pcbM->lastack);
    }
    packet->insertAtFront(tcpHdr);

//    auto payload = makeShared<BytesChunk>((const uint8_t*)tcpDataP, tcpLengthP);
//    const auto& tcpHdr = payload->Chunk::peek<TcpHeader>(byte(0));
//    payload->removeFromBeginning(tcpHdr->getChunkLength());

    char msgname[80];
    snprintf(msgname, sizeof(msgname), "%.10s%s%s%s(l=%lu bytes)",
            "tcpHdr",
            tcpHdr->getSynBit() ? " SYN" : "",
            tcpHdr->getFinBit() ? " FIN" : "",
            (tcpHdr->getAckBit() && 0 == numBytes) ? " ACK" : "",
            (unsigned long)numBytes);

    return packet;
}

void TcpLwipSendQueue::discardAckedBytes(unsigned long bytesP)
{
    // nothing to do here
}

TcpLwipReceiveQueue::TcpLwipReceiveQueue()
{
}

TcpLwipReceiveQueue::~TcpLwipReceiveQueue()
{
    // nothing to do here
}

void TcpLwipReceiveQueue::setConnection(TcpLwipConnection *connP)
{
    ASSERT(connP);
    ASSERT(connM == nullptr);

    dataBuffer.clear();
    taggedSegments.clear();
    connM = connP;
}

void TcpLwipReceiveQueue::notifyAboutIncomingSegmentProcessing(Packet *packet, uint32_t seqno, const void *bufferP, size_t bufferLengthP)
{
    ASSERT(packet);
    ASSERT(bufferP);

    // Cache the region tags carried by this segment's payload, keyed by its TCP
    // sequence number, so they can be re-attached when lwIP delivers the
    // (possibly reassembled) data. Called for every arriving data segment,
    // in-order or out-of-order.
    if (bufferLengthP > 0) {
        const auto& tcpHdr = packet->peekAtFront<TcpHeader>();
        taggedSegments[seqno] = packet->peekAt(tcpHdr->getChunkLength(), B(bufferLengthP));
    }
}

const Ptr<const Chunk> TcpLwipReceiveQueue::getCachedTaggedData(uint32_t seqNoP, unsigned int lengthP)
{
    return assembleTaggedData(taggedSegments, seqNoP, lengthP);
}

void TcpLwipReceiveQueue::enqueueTcpLayerData(void *dataP, unsigned int dataLengthP, uint32_t seqNoP)
{
    // Prefer the region-tagged application data cached as the segments arrived;
    // fall back to plain bytes if the range is not (fully) cached (e.g. a tag-less
    // transport on the far side).
    Ptr<const Chunk> chunk = getCachedTaggedData(seqNoP, dataLengthP);
    if (chunk == nullptr)
        chunk = makeShared<BytesChunk>(static_cast<uint8_t *>(dataP), dataLengthP);
    dataBuffer.push(chunk);
    pruneTaggedData(taggedSegments, seqNoP + dataLengthP);
}

B TcpLwipReceiveQueue::getExtractableBytesUpTo() const
{
    return B(dataBuffer.getLength());
}

Packet *TcpLwipReceiveQueue::extractBytesUpTo(B length)
{
    ASSERT(connM);

    Packet *dataMsg = nullptr;
    b queueLength = dataBuffer.getLength();

    if (queueLength > b(0)) {
        dataMsg = new Packet("DATA", TCP_I_DATA);
        const auto& data = dataBuffer.pop<Chunk>(queueLength);
        dataMsg->insertAtBack(data);
    }

    return dataMsg;
}

uint32_t TcpLwipReceiveQueue::getAmountOfBufferedBytes() const
{
    return dataBuffer.getLength().get<B>();
}

uint32_t TcpLwipReceiveQueue::getQueueLength() const
{
    return dataBuffer.getLength().get<B>();
}

void TcpLwipReceiveQueue::getQueueStatus() const
{
    // TODO
}

void TcpLwipReceiveQueue::notifyAboutSending(const TcpHeader *tcpsegP)
{
    // nothing to do
}

} // namespace tcp
} // namespace inet

