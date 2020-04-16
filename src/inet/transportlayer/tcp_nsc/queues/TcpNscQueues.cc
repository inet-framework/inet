//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2010 Zoltan Bojthe
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

#include "inet/transportlayer/tcp_nsc/queues/TcpNscQueues.h"

#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/tcp_nsc/TcpNscConnection.h"

namespace inet {

namespace tcp {

Register_Class(TcpNscSendQueue);

Register_Class(TcpNscReceiveQueue);

TcpNscSendQueue::TcpNscSendQueue()
{
}

TcpNscSendQueue::~TcpNscSendQueue()
{
}

void TcpNscSendQueue::setConnection(TcpNscConnection *connP)
{
    dataBuffer.clear();
    connM = connP;
}

void TcpNscSendQueue::enqueueAppData(Packet *msg)
{
    ASSERT(msg);
    dataBuffer.push(msg->peekDataAt(B(0), B(msg->getByteLength())));
    delete msg;
}

int TcpNscSendQueue::getBytesForTcpLayer(void *bufferP, int bufferLengthP) const
{
    ASSERT(bufferP);

    int length = B(dataBuffer.getLength()).get();
    if (bufferLengthP < length)
        length = bufferLengthP;
    if (length == 0)
        return 0;

    const auto& bytesChunk = dataBuffer.peek<BytesChunk>(B(length));
    return bytesChunk->copyToBuffer(static_cast<uint8_t*>(bufferP), length);
}

void TcpNscSendQueue::dequeueTcpLayerMsg(int msgLengthP)
{
    dataBuffer.pop(B(msgLengthP));
}

ulong TcpNscSendQueue::getBytesAvailable() const
{
    return B(dataBuffer.getLength()).get();
}

Packet *TcpNscSendQueue::createSegmentWithBytes(const void *tcpDataP, int tcpLengthP)
{
    ASSERT(tcpDataP);

    const auto& bytes = makeShared<BytesChunk>(static_cast<const uint8_t*>(tcpDataP), tcpLengthP);
    auto packet = new Packet(nullptr, bytes);
    const auto& tcpHdr = packet->popAtFront<TcpHeader>();
    packet->trimFront();
    int64_t numBytes = packet->getByteLength();
    packet->insertAtFront(tcpHdr);

//    auto payload = makeShared<BytesChunk>((const uint8_t*)tcpDataP, tcpLengthP);
//    const auto& tcpHdr = (payload->Chunk::peek<TcpHeader>(byte(0)));
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

void TcpNscSendQueue::discardUpTo(uint32 seqNumP)
{
    // nothing to do here
}

////////////////////////////////////////////////////////////////////////////////////////

TcpNscReceiveQueue::TcpNscReceiveQueue()
{
}

TcpNscReceiveQueue::~TcpNscReceiveQueue()
{
    // nothing to do here
}

void TcpNscReceiveQueue::setConnection(TcpNscConnection *connP)
{
    ASSERT(connP);

    dataBuffer.clear();
    connM = connP;
}

void TcpNscReceiveQueue::notifyAboutIncomingSegmentProcessing(Packet *packet)
{
    ASSERT(packet);
}

void TcpNscReceiveQueue::enqueueNscData(void *dataP, int dataLengthP)
{
    dataBuffer.push(makeShared<BytesChunk>(static_cast<uint8_t *>(dataP), dataLengthP));
}

Packet *TcpNscReceiveQueue::extractBytesUpTo()
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

uint32 TcpNscReceiveQueue::getAmountOfBufferedBytes() const
{
    return B(dataBuffer.getLength()).get();
}

uint32 TcpNscReceiveQueue::getQueueLength() const
{
    return B(dataBuffer.getLength()).get();
}

void TcpNscReceiveQueue::getQueueStatus() const
{
    // TODO
}

void TcpNscReceiveQueue::notifyAboutSending(const Packet *packet)
{
    // nothing to do
}

} // namespace tcp

} // namespace inet

