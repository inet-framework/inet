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

#include "inet/transportlayer/tcp_nsc/queues/TCP_NSC_Queues.h"

#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "inet/transportlayer/tcp_common/TCPSegment.h"
#include "inet/transportlayer/tcp_nsc/TCP_NSC_Connection.h"

namespace inet {

namespace tcp {

Register_Class(TCP_NSC_SendQueue);

Register_Class(TCP_NSC_ReceiveQueue);

TCP_NSC_SendQueue::TCP_NSC_SendQueue()
{
}

TCP_NSC_SendQueue::~TCP_NSC_SendQueue()
{
}

void TCP_NSC_SendQueue::setConnection(TCP_NSC_Connection *connP)
{
    dataBuffer.clear();
    connM = connP;
}

void TCP_NSC_SendQueue::enqueueAppData(Packet *msg)
{
    ASSERT(msg);
    dataBuffer.push(msg->peekDataAt(byte(0), byte(msg->getByteLength())));
    delete msg;
}

int TCP_NSC_SendQueue::getBytesForTcpLayer(void *bufferP, int bufferLengthP) const
{
    ASSERT(bufferP);

    unsigned int length = byte(dataBuffer.getLength()).get();
    if (bufferLengthP < length)
        length = bufferLengthP;
    if (length == 0)
        return 0;

    const auto& bytesChunk = dataBuffer.peek<BytesChunk>(byte(length));
    return bytesChunk->copyToBuffer((uint8_t*)bufferP, length);
}

void TCP_NSC_SendQueue::dequeueTcpLayerMsg(int msgLengthP)
{
    dataBuffer.pop(byte(msgLengthP));
}

ulong TCP_NSC_SendQueue::getBytesAvailable() const
{
    return byte(dataBuffer.getLength()).get();
}

Packet *TCP_NSC_SendQueue::createSegmentWithBytes(const void *tcpDataP, int tcpLengthP)
{
    ASSERT(tcpDataP);

    const auto& bytes = std::make_shared<BytesChunk>((const uint8_t*)tcpDataP, tcpLengthP);
    bytes->markImmutable();
    auto packet = new Packet(nullptr, bytes);
    const auto& tcpHdr = packet->popHeader<TcpHeader>();
    packet->removePoppedHeaders();
    int64_t numBytes = packet->getByteLength();
    packet->pushHeader(tcpHdr);

//    auto payload = std::make_shared<BytesChunk>((const uint8_t*)tcpDataP, tcpLengthP);
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

void TCP_NSC_SendQueue::discardUpTo(uint32 seqNumP)
{
    // nothing to do here
}

////////////////////////////////////////////////////////////////////////////////////////

TCP_NSC_ReceiveQueue::TCP_NSC_ReceiveQueue()
{
}

TCP_NSC_ReceiveQueue::~TCP_NSC_ReceiveQueue()
{
    // nothing to do here
}

void TCP_NSC_ReceiveQueue::setConnection(TCP_NSC_Connection *connP)
{
    ASSERT(connP);

    dataBuffer.clear();
    connM = connP;
}

void TCP_NSC_ReceiveQueue::notifyAboutIncomingSegmentProcessing(Packet *packet)
{
    ASSERT(packet);
}

void TCP_NSC_ReceiveQueue::enqueueNscData(void *dataP, int dataLengthP)
{
    const auto& bytes = std::make_shared<BytesChunk>((uint8_t *)dataP, dataLengthP);
    bytes->markImmutable();
    dataBuffer.push(bytes);
}

cPacket *TCP_NSC_ReceiveQueue::extractBytesUpTo()
{
    ASSERT(connM);

    Packet *dataMsg = nullptr;
    bit queueLength = dataBuffer.getLength();

    if (queueLength > bit(0)) {
        dataMsg = new Packet("DATA");
        dataMsg->setKind(TCP_I_DATA);
        const auto& data = dataBuffer.pop<Chunk>(queueLength);
        dataMsg->append(data);
    }

    return dataMsg;
}

uint32 TCP_NSC_ReceiveQueue::getAmountOfBufferedBytes() const
{
    return byte(dataBuffer.getLength()).get();
}

uint32 TCP_NSC_ReceiveQueue::getQueueLength() const
{
    return byte(dataBuffer.getLength()).get();
}

void TCP_NSC_ReceiveQueue::getQueueStatus() const
{
    // TODO
}

void TCP_NSC_ReceiveQueue::notifyAboutSending(const Packet *packet)
{
    // nothing to do
}

} // namespace tcp

} // namespace inet

