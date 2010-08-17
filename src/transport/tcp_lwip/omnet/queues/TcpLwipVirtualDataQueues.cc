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

#include "TcpLwipVirtualDataQueues.h"

#include "TCPCommand.h"
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

int TcpLwipVirtualDataSendQueue::getBytesForTcpLayer(void* bufferP, int bufferLengthP)
{
    ASSERT(bufferP);

    return (unsentTcpLayerBytesM > bufferLengthP) ? bufferLengthP : unsentTcpLayerBytesM;
}

void TcpLwipVirtualDataSendQueue::dequeueTcpLayerMsg(int msgLengthP)
{
    ASSERT(msgLengthP <= unsentTcpLayerBytesM);

    unsentTcpLayerBytesM -= msgLengthP;
}

ulong TcpLwipVirtualDataSendQueue::getBytesAvailable()
{
    return unsentTcpLayerBytesM; // TODO
}

TCPSegment* TcpLwipVirtualDataSendQueue::createSegmentWithBytes(
        const void* tcpDataP, int tcpLengthP)
{
    ASSERT(tcpDataP);

    TCPSegment *tcpseg = new TCPSegment("");

    TCPSerializer().parse((const unsigned char *)tcpDataP, tcpLengthP, tcpseg);
    uint32 numBytes = tcpseg->getPayloadLength();

    char msgname[80];
    sprintf(msgname, "%.10s%s%s%s(l=%lu,%dmsg)",
            "tcpseg",
            tcpseg->getSynBit() ? " SYN":"",
            tcpseg->getFinBit() ? " FIN":"",
            (tcpseg->getAckBit() && 0==numBytes) ? " ACK":"",
            (unsigned long)numBytes,
            tcpseg->getPayloadArraySize());
    tcpseg->setName(msgname);

    return tcpseg;
}

void TcpLwipVirtualDataSendQueue::discardAckedBytes(unsigned long bytesP)
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

void TcpLwipVirtualDataReceiveQueue::insertBytesFromSegment(
        TCPSegment *tcpsegP, uint32 seqno, void* bufferP, size_t bufferLengthP)
{
    ASSERT(tcpsegP);
    ASSERT(bufferP);

//    return TCPSerializer().serialize(tcpsegP, (unsigned char *)bufferP, bufferLengthP);
}

void TcpLwipVirtualDataReceiveQueue::enqueueTcpLayerData(void* dataP, int dataLengthP)
{
    bytesInQueueM += dataLengthP;
}

long TcpLwipVirtualDataReceiveQueue::getExtractableBytesUpTo()
{
    return bytesInQueueM;
}

TCPDataMsg* TcpLwipVirtualDataReceiveQueue::extractBytesUpTo(long maxBytesP)
{
    ASSERT(connM);

    TCPDataMsg *dataMsg = NULL;
    if(bytesInQueueM && maxBytesP)
    {
        dataMsg = new TCPDataMsg("DATA");
        dataMsg->setKind(TCP_I_DATA);
        dataMsg->setByteLength(std::min(bytesInQueueM, maxBytesP));
        dataMsg->setDataObject(NULL);
        dataMsg->setIsBegin(false);
        bytesInQueueM -= dataMsg->getByteLength();
    }
    return dataMsg;
}

uint32 TcpLwipVirtualDataReceiveQueue::getAmountOfBufferedBytes()
{
    return bytesInQueueM;
}

uint32 TcpLwipVirtualDataReceiveQueue::getQueueLength()
{
    return bytesInQueueM ? 1 : 0;
}

void TcpLwipVirtualDataReceiveQueue::getQueueStatus()
{
    // TODO
}

void TcpLwipVirtualDataReceiveQueue::notifyAboutSending(const TCPSegment *tcpsegP)
{
    // nothing to do
}
