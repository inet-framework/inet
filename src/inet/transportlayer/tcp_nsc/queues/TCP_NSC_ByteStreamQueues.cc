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

#include "inet/transportlayer/tcp_nsc/queues/TCP_NSC_ByteStreamQueues.h"

#include "inet/common/ByteArrayMessage.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "inet/transportlayer/tcp_nsc/TCP_NSC_Connection.h"
#include "inet/common/serializer/tcp/TCPSerializer.h"
#include "inet/transportlayer/tcp_common/TCPSegment.h"

namespace inet {

namespace tcp {

Register_Class(TCP_NSC_ByteStreamSendQueue);

Register_Class(TCP_NSC_ByteStreamReceiveQueue);

TCP_NSC_ByteStreamSendQueue::TCP_NSC_ByteStreamSendQueue()
{
}

TCP_NSC_ByteStreamSendQueue::~TCP_NSC_ByteStreamSendQueue()
{
}

void TCP_NSC_ByteStreamSendQueue::setConnection(TCP_NSC_Connection *connP)
{
    byteArrayBufferM.clear();
    TCP_NSC_SendQueue::setConnection(connP);
}

void TCP_NSC_ByteStreamSendQueue::enqueueAppData(cPacket *msgP)
{
    ASSERT(msgP);

    ByteArrayMessage *msg = check_and_cast<ByteArrayMessage *>(msgP);
    int64 bytes = msg->getByteLength();
    ASSERT(bytes == msg->getByteArray().getDataArraySize());
    byteArrayBufferM.push(msg->getByteArray());
    delete msgP;
}

int TCP_NSC_ByteStreamSendQueue::getBytesForTcpLayer(void *bufferP, int bufferLengthP) const
{
    ASSERT(bufferP);
    return byteArrayBufferM.getBytesToBuffer(bufferP, bufferLengthP);
}

void TCP_NSC_ByteStreamSendQueue::dequeueTcpLayerMsg(int msgLengthP)
{
    byteArrayBufferM.drop(msgLengthP);
}

ulong TCP_NSC_ByteStreamSendQueue::getBytesAvailable() const
{
    return byteArrayBufferM.getLength();
}

TCPSegment *TCP_NSC_ByteStreamSendQueue::createSegmentWithBytes(const void *tcpDataP, int tcpLengthP)
{
    ASSERT(tcpDataP);

    TCPSegment *tcpseg = new TCPSegment();

    TCPSerializer().parse((const unsigned char *)tcpDataP, tcpLengthP, tcpseg, true);
    uint32 numBytes = tcpseg->getPayloadLength();

    char msgname[80];
    sprintf(msgname, "%.10s%s%s%s(l=%lu,%u bytes)",
            "tcpseg",
            tcpseg->getSynBit() ? " SYN" : "",
            tcpseg->getFinBit() ? " FIN" : "",
            (tcpseg->getAckBit() && 0 == numBytes) ? " ACK" : "",
            (unsigned long)numBytes,
            tcpseg->getByteArray().getDataArraySize());
    tcpseg->setName(msgname);

    return tcpseg;
}

void TCP_NSC_ByteStreamSendQueue::discardUpTo(uint32 seqNumP)
{
    // nothing to do here
}

////////////////////////////////////////////////////////////////////////////////////////

TCP_NSC_ByteStreamReceiveQueue::TCP_NSC_ByteStreamReceiveQueue()
{
}

TCP_NSC_ByteStreamReceiveQueue::~TCP_NSC_ByteStreamReceiveQueue()
{
    // nothing to do here
}

void TCP_NSC_ByteStreamReceiveQueue::setConnection(TCP_NSC_Connection *connP)
{
    ASSERT(connP);

    byteArrayBufferM.clear();
    TCP_NSC_ReceiveQueue::setConnection(connP);
}

void TCP_NSC_ByteStreamReceiveQueue::notifyAboutIncomingSegmentProcessing(TCPSegment *tcpsegP)
{
    ASSERT(tcpsegP);
}

void TCP_NSC_ByteStreamReceiveQueue::enqueueNscData(void *dataP, int dataLengthP)
{
    byteArrayBufferM.push(dataP, dataLengthP);
}

cPacket *TCP_NSC_ByteStreamReceiveQueue::extractBytesUpTo()
{
    ASSERT(connM);

    ByteArrayMessage *dataMsg = NULL;
    uint64 bytesInQueue = byteArrayBufferM.getLength();

    if (bytesInQueue) {
        dataMsg = new ByteArrayMessage("DATA");
        dataMsg->setKind(TCP_I_DATA);
        unsigned int extractBytes = bytesInQueue;
        char *data = new char[extractBytes];
        unsigned int extractedBytes = byteArrayBufferM.popBytesToBuffer(data, extractBytes);
        dataMsg->setByteLength(extractedBytes);
        dataMsg->getByteArray().assignBuffer(data, extractedBytes);
    }

    return dataMsg;
}

uint32 TCP_NSC_ByteStreamReceiveQueue::getAmountOfBufferedBytes() const
{
    return byteArrayBufferM.getLength();
}

uint32 TCP_NSC_ByteStreamReceiveQueue::getQueueLength() const
{
    return byteArrayBufferM.getLength();
}

void TCP_NSC_ByteStreamReceiveQueue::getQueueStatus() const
{
    // TODO
}

void TCP_NSC_ByteStreamReceiveQueue::notifyAboutSending(const TCPSegment *tcpsegP)
{
    // nothing to do
}

} // namespace tcp

} // namespace inet

