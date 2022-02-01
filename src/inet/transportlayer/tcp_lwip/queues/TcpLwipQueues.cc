//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/tcp_lwip/queues/TcpLwipQueues.h"

#include "inet/common/packet/serializer/BytesChunkSerializer.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/tcp_common/TcpHeaderSerializer.h"
#include "inet/transportlayer/tcp_lwip/TcpLwipConnection.h"

namespace inet {
namespace tcp {

Register_Class(TcpLwipSendQueue);

Register_Class(TcpLwipReceiveQueue);

TcpLwipSendQueue::TcpLwipSendQueue()
{
}

TcpLwipSendQueue::~TcpLwipSendQueue()
{
}

void TcpLwipSendQueue::setConnection(TcpLwipConnection *connP)
{
    dataBuffer.clear();
    connM = connP;
}

void TcpLwipSendQueue::enqueueAppData(Packet *msg)
{
    ASSERT(msg);
    dataBuffer.push(msg->peekDataAt(B(0), B(msg->getByteLength())));
    delete msg;
}

unsigned int TcpLwipSendQueue::getBytesForTcpLayer(void *bufferP, unsigned int bufferLengthP) const
{
    ASSERT(bufferP);

    unsigned int length = B(dataBuffer.getLength()).get();
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
    return B(dataBuffer.getLength()).get();
}

Packet *TcpLwipSendQueue::createSegmentWithBytes(const void *tcpDataP, unsigned int tcpLengthP)
{
    ASSERT(tcpDataP);

    const auto& bytes = makeShared<BytesChunk>((const uint8_t *)tcpDataP, tcpLengthP);
    auto packet = new Packet(nullptr, bytes);
    auto tcpHdr = packet->removeAtFront<TcpHeader>();
    int64_t numBytes = packet->getByteLength();
    packet->insertAtFront(tcpHdr);

//    auto payload = makeShared<BytesChunk>((const uint8_t*)tcpDataP, tcpLengthP);
//    const auto& tcpHdr = payload->Chunk::peek<TcpHeader>(byte(0));
//    payload->removeFromBeginning(tcpHdr->getChunkLength());

    char msgname[80];
    sprintf(msgname, "%.10s%s%s%s(l=%lu bytes)",
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
    connM = connP;
}

void TcpLwipReceiveQueue::notifyAboutIncomingSegmentProcessing(Packet *packet, uint32_t seqno, const void *bufferP, size_t bufferLengthP)
{
    ASSERT(packet);
    ASSERT(bufferP);
}

void TcpLwipReceiveQueue::enqueueTcpLayerData(void *dataP, unsigned int dataLengthP)
{
    dataBuffer.push(makeShared<BytesChunk>(static_cast<uint8_t *>(dataP), dataLengthP));
}

unsigned long TcpLwipReceiveQueue::getExtractableBytesUpTo() const
{
    return B(dataBuffer.getLength()).get();
}

Packet *TcpLwipReceiveQueue::extractBytesUpTo()
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
    return B(dataBuffer.getLength()).get();
}

uint32_t TcpLwipReceiveQueue::getQueueLength() const
{
    return B(dataBuffer.getLength()).get();
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

