//
// Copyright (C) 2004 Andras Varga
//               2010 Zoltan Bojthe
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


#include "TcpLwipByteStreamQueues.h"

#include "ByteArrayMessage.h"
#include "TCPCommand_m.h"
#include "TcpLwipConnection.h"
#include "TCPSegment.h"
#include "TCPSerializer.h"


Register_Class(TcpLwipByteStreamSendQueue);

Register_Class(TcpLwipByteStreamReceiveQueue);


TcpLwipByteStreamSendQueue::TcpLwipByteStreamSendQueue()
{
}

TcpLwipByteStreamSendQueue::~TcpLwipByteStreamSendQueue()
{
}

void TcpLwipByteStreamSendQueue::setConnection(TcpLwipConnection *connP)
{
    byteArrayBufferM.clear();
    TcpLwipSendQueue::setConnection(connP);
}

void TcpLwipByteStreamSendQueue::enqueueAppData(cPacket *msgP)
{
    ASSERT(msgP);

    ByteArrayMessage *msg = check_and_cast<ByteArrayMessage *>(msgP);
    int64 bytes = msg->getByteLength();

    ASSERT(bytes == msg->getByteArray().getDataArraySize());

    byteArrayBufferM.push(msg->getByteArray());
    delete msgP;
}

unsigned int TcpLwipByteStreamSendQueue::getBytesForTcpLayer(
        void* bufferP, unsigned int bufferLengthP) const
{
    ASSERT(bufferP);

    return byteArrayBufferM.getBytesToBuffer(bufferP, bufferLengthP);
}

void TcpLwipByteStreamSendQueue::dequeueTcpLayerMsg(unsigned int msgLengthP)
{
    byteArrayBufferM.drop(msgLengthP);
}

unsigned long TcpLwipByteStreamSendQueue::getBytesAvailable() const
{
    return byteArrayBufferM.getLength();
}

TCPSegment* TcpLwipByteStreamSendQueue::createSegmentWithBytes(
        const void* tcpDataP, unsigned int tcpLengthP)
{
    ASSERT(tcpDataP);

    TCPSegment *tcpseg = new TCPSegment();

    TCPSerializer().parse((const unsigned char *)tcpDataP, tcpLengthP, tcpseg, true);
    uint32 numBytes = tcpseg->getPayloadLength();

    char msgname[80];

    sprintf(msgname, "%.10s%s%s%s(l=%lu,%u bytes)",
            "tcpseg",
            tcpseg->getSynBit() ? " SYN":"",
            tcpseg->getFinBit() ? " FIN":"",
            (tcpseg->getAckBit() && 0 == numBytes) ? " ACK":"",
            (unsigned long)numBytes,
            tcpseg->getByteArray().getDataArraySize());
    tcpseg->setName(msgname);

    return tcpseg;
}

void TcpLwipByteStreamSendQueue::discardAckedBytes(unsigned long bytesP)
{
    // nothing to do here
}

TcpLwipByteStreamReceiveQueue::TcpLwipByteStreamReceiveQueue()
{
}

TcpLwipByteStreamReceiveQueue::~TcpLwipByteStreamReceiveQueue()
{
    // nothing to do here
}

void TcpLwipByteStreamReceiveQueue::setConnection(TcpLwipConnection *connP)
{
    ASSERT(connP);

    byteArrayBufferM.clear();
    TcpLwipReceiveQueue::setConnection(connP);
}

void TcpLwipByteStreamReceiveQueue::notifyAboutIncomingSegmentProcessing(
        TCPSegment *tcpsegP, uint32 seqno, const void* bufferP, size_t bufferLengthP)
{
    ASSERT(tcpsegP);
    ASSERT(bufferP);
}

void TcpLwipByteStreamReceiveQueue::enqueueTcpLayerData(void* dataP, unsigned int dataLengthP)
{
    byteArrayBufferM.push(dataP, dataLengthP);
}

unsigned long TcpLwipByteStreamReceiveQueue::getExtractableBytesUpTo() const
{
    return byteArrayBufferM.getLength();
}

cPacket* TcpLwipByteStreamReceiveQueue::extractBytesUpTo()
{
    ASSERT(connM);

    ByteArrayMessage *dataMsg = NULL;
    uint64 bytesInQueue = byteArrayBufferM.getLength();

    if (bytesInQueue)
    {
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

uint32 TcpLwipByteStreamReceiveQueue::getAmountOfBufferedBytes() const
{
    return byteArrayBufferM.getLength();
}

uint32 TcpLwipByteStreamReceiveQueue::getQueueLength() const
{
    return byteArrayBufferM.getLength();
}

void TcpLwipByteStreamReceiveQueue::getQueueStatus() const
{
    // TODO
}

void TcpLwipByteStreamReceiveQueue::notifyAboutSending(const TCPSegment *tcpsegP)
{
    // nothing to do
}
