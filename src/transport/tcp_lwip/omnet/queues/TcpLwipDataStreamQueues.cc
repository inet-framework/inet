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


#include <omnetpp.h>

#include "TcpLwipDataStreamQueues.h"

#include "TCPCommand.h"
#include "TcpLwipConnection.h"
#include "TCPSegmentWithData.h"
#include "TCPSerializer.h"


Register_Class(TcpLwipDataStreamSendQueue);

Register_Class(TcpLwipDataStreamReceiveQueue);


TcpLwipDataStreamSendQueue::TcpLwipDataStreamSendQueue()
{
}

TcpLwipDataStreamSendQueue::~TcpLwipDataStreamSendQueue()
{
}

void TcpLwipDataStreamSendQueue::setConnection(TcpLwipConnection *connP)
{
    byteArrayListM.clear();
    TcpLwipSendQueue::setConnection(connP);
}

void TcpLwipDataStreamSendQueue::enqueueAppData(cPacket *msgP)
{
    ASSERT(msgP);

    ByteArrayMessage *msg = check_and_cast<ByteArrayMessage *>(msgP);
    int64 bytes = msg->getByteLength();
    ASSERT (bytes == msg->getByteArray().getDataArraySize());
    byteArrayListM.push(msg->getByteArray());
}

unsigned int TcpLwipDataStreamSendQueue::getBytesForTcpLayer(void* bufferP, unsigned int bufferLengthP)
{
    ASSERT(bufferP);

    return byteArrayListM.getBytesToBuffer(bufferP, bufferLengthP);
}

void TcpLwipDataStreamSendQueue::dequeueTcpLayerMsg(unsigned int msgLengthP)
{
    byteArrayListM.drop(msgLengthP);
}

ulong TcpLwipDataStreamSendQueue::getBytesAvailable()
{
    return byteArrayListM.getLength();
}

TCPSegment* TcpLwipDataStreamSendQueue::createSegmentWithBytes(
        const void* tcpDataP, unsigned int tcpLengthP)
{
    ASSERT(tcpDataP);

    TCPSegmentWithBytes *tcpseg = new TCPSegmentWithBytes("");

    TCPSerializer().parse((const unsigned char *)tcpDataP, tcpLengthP, tcpseg);
    uint32 numBytes = tcpseg->getPayloadLength();

    char msgname[80];
    sprintf(msgname, "%.10s%s%s%s(l=%lu,%u bytes)",
            "tcpseg",
            tcpseg->getSynBit() ? " SYN":"",
            tcpseg->getFinBit() ? " FIN":"",
            (tcpseg->getAckBit() && 0==numBytes) ? " ACK":"",
            (unsigned long)numBytes,
            tcpseg->getByteArray().getDataArraySize());
    tcpseg->setName(msgname);

    return tcpseg;
}

void TcpLwipDataStreamSendQueue::discardAckedBytes(unsigned long bytesP)
{
    // nothing to do here
}

TcpLwipDataStreamReceiveQueue::TcpLwipDataStreamReceiveQueue()
{
}

TcpLwipDataStreamReceiveQueue::~TcpLwipDataStreamReceiveQueue()
{
    // nothing to do here
}

void TcpLwipDataStreamReceiveQueue::setConnection(TcpLwipConnection *connP)
{
    ASSERT(connP);

    byteArrayListM.clear();
    TcpLwipReceiveQueue::setConnection(connP);
}

void TcpLwipDataStreamReceiveQueue::insertBytesFromSegment(
        TCPSegment *tcpsegP, uint32 seqno, void* bufferP, size_t bufferLengthP)
{
    ASSERT(tcpsegP);
    ASSERT(bufferP);

//    return TCPSerializer().serialize(tcpsegP, (unsigned char *)bufferP, bufferLengthP);
}

void TcpLwipDataStreamReceiveQueue::enqueueTcpLayerData(void* dataP, unsigned int dataLengthP)
{
    ByteArray byteArray;

    byteArray.setDataFromBuffer(dataP, dataLengthP);
    byteArrayListM.push(byteArray);
}

unsigned long TcpLwipDataStreamReceiveQueue::getExtractableBytesUpTo()
{
    return byteArrayListM.getLength();
}

TCPDataMsg* TcpLwipDataStreamReceiveQueue::extractBytesUpTo(unsigned long maxBytesP)
{
    ASSERT(connM);

    TCPDataMsg *dataMsg = NULL;
    uint64 bytesInQueue = byteArrayListM.getLength();
    if(bytesInQueue && maxBytesP)
    {
        dataMsg = new TCPDataMsg("DATA");
        dataMsg->setKind(TCP_I_DATA);
        unsigned int extractBytes = bytesInQueue > maxBytesP ? maxBytesP : bytesInQueue;
        char *data = new char[extractBytes];
        unsigned int extractedBytes = byteArrayListM.popBytesToBuffer(data, extractBytes);
        dataMsg->setByteLength(extractedBytes);
        dataMsg->setDataFromBuffer(data, extractedBytes);
        delete data;
    }
    return dataMsg;
}

uint32 TcpLwipDataStreamReceiveQueue::getAmountOfBufferedBytes()
{
    return byteArrayListM.getLength();
}

uint32 TcpLwipDataStreamReceiveQueue::getQueueLength()
{
    return byteArrayListM.getLength();
}

void TcpLwipDataStreamReceiveQueue::getQueueStatus()
{
    // TODO
}

void TcpLwipDataStreamReceiveQueue::notifyAboutSending(const TCPSegment *tcpsegP)
{
    // nothing to do
}
