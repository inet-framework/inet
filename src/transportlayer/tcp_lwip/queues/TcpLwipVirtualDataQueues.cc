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


#include "TcpLwipVirtualDataQueues.h"

#include "TCPCommand_m.h"
#include "TcpLwipConnection.h"
#include "TCPSerializer.h"


Register_Class(TcpLwipVirtualDataSendQueue);

Register_Class(TcpLwipVirtualDataReceiveQueue);


TcpLwipVirtualDataSendQueue::TcpLwipVirtualDataSendQueue()
    :
    unsentTcpLayerBytesM(0)
{
}

TcpLwipVirtualDataSendQueue::~TcpLwipVirtualDataSendQueue()
{
}

void TcpLwipVirtualDataSendQueue::setConnection(TcpLwipConnection *connP)
{
    unsentTcpLayerBytesM = 0;
    TcpLwipSendQueue::setConnection(connP);
}

void TcpLwipVirtualDataSendQueue::enqueueAppData(cPacket *msgP)
{
    ASSERT(msgP);

    int bytes = msgP->getByteLength();
    delete msgP;

    unsentTcpLayerBytesM += bytes;
}

unsigned int TcpLwipVirtualDataSendQueue::getBytesForTcpLayer(
        void* bufferP, unsigned int bufferLengthP) const
{
    ASSERT(bufferP);

    return (unsentTcpLayerBytesM > bufferLengthP) ? bufferLengthP : unsentTcpLayerBytesM;
}

void TcpLwipVirtualDataSendQueue::dequeueTcpLayerMsg(unsigned int msgLengthP)
{
    ASSERT(msgLengthP <= unsentTcpLayerBytesM);

    unsentTcpLayerBytesM -= msgLengthP;
}

unsigned long TcpLwipVirtualDataSendQueue::getBytesAvailable() const
{
    return unsentTcpLayerBytesM;
}

TCPSegment* TcpLwipVirtualDataSendQueue::createSegmentWithBytes(
        const void* tcpDataP, unsigned int tcpLengthP)
{
    ASSERT(tcpDataP);

    TCPSegment *tcpseg = new TCPSegment("");

    TCPSerializer().parse((const unsigned char *)tcpDataP, tcpLengthP, tcpseg, false);
    uint32 numBytes = tcpseg->getPayloadLength();

    char msgname[80];
    sprintf(msgname, "%.10s%s%s%s(l=%lu)",
            "tcpseg",
            tcpseg->getSynBit() ? " SYN" : "",
            tcpseg->getFinBit() ? " FIN" : "",
            (tcpseg->getAckBit() && 0 == numBytes) ? " ACK" : "",
            (unsigned long)numBytes);
    tcpseg->setName(msgname);

    return tcpseg;
}

void TcpLwipVirtualDataSendQueue::discardUpTo(uint32 seqNumP)
{
    // nothing to do here
}

TcpLwipVirtualDataReceiveQueue::TcpLwipVirtualDataReceiveQueue()
    :
    bytesInQueueM(0)
{
}

TcpLwipVirtualDataReceiveQueue::~TcpLwipVirtualDataReceiveQueue()
{
    // nothing to do here
}

void TcpLwipVirtualDataReceiveQueue::setConnection(TcpLwipConnection *connP)
{
    ASSERT(connP);

    bytesInQueueM = 0;
    TcpLwipReceiveQueue::setConnection(connP);
}

void TcpLwipVirtualDataReceiveQueue::notifyAboutIncomingSegmentProcessing(
        TCPSegment *tcpsegP, uint32 seqno, const void *bufferP, size_t bufferLengthP)
{
    ASSERT(tcpsegP);
    ASSERT(bufferP);
}

void TcpLwipVirtualDataReceiveQueue::enqueueTcpLayerData(void* dataP, unsigned int dataLengthP)
{
    bytesInQueueM += dataLengthP;
}

cPacket* TcpLwipVirtualDataReceiveQueue::extractBytesUpTo()
{
    ASSERT(connM);

    cPacket *dataMsg = NULL;

    if (bytesInQueueM)
    {
        dataMsg = new cPacket("DATA");
        dataMsg->setKind(TCP_I_DATA);
        dataMsg->setByteLength(bytesInQueueM);
        bytesInQueueM -= dataMsg->getByteLength();
    }

    return dataMsg;
}

uint32 TcpLwipVirtualDataReceiveQueue::getAmountOfBufferedBytes() const
{
    return bytesInQueueM;
}

uint32 TcpLwipVirtualDataReceiveQueue::getQueueLength() const
{
    return bytesInQueueM ? 1 : 0;
}

void TcpLwipVirtualDataReceiveQueue::getQueueStatus() const
{
    // TODO
}

void TcpLwipVirtualDataReceiveQueue::notifyAboutSending(const TCPSegment *tcpsegP)
{
    // nothing to do
}
