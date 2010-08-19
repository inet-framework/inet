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
#include "TCPSerializer.h"


Register_Class(TcpLwipDataStreamSendQueue);

Register_Class(TcpLwipDataStreamReceiveQueue);


TcpLwipDataStreamSendQueue::TcpLwipDataStreamSendQueue()
    :
    enquedBytesM(0), dequedBytesM(0)
{
}

TcpLwipDataStreamSendQueue::~TcpLwipDataStreamSendQueue()
{
}

void TcpLwipDataStreamSendQueue::setConnection(TcpLwipConnection *connP)
{
    enquedBytesM = dequedBytesM = 0;
    TcpLwipSendQueue::setConnection(connP);
}

void TcpLwipDataStreamSendQueue::enqueueAppData(cPacket *msgP)
{
    ASSERT(msgP);

    int bytes = msgP->getByteLength();
    enquedBytesM += bytes;
    Payload payload;
    payload.endStreamOffset = enquedBytesM;
    payload.msg = msgP;
    payloadQueueM.push_back(payload);
}

unsigned int TcpLwipDataStreamSendQueue::getBytesForTcpLayer(void* bufferP, unsigned int bufferLengthP)
{
    ASSERT(bufferP);

    uint64 unsentTcpLayerBytes = enquedBytesM - dequedBytesM;
    int bytes = (unsentTcpLayerBytes > bufferLengthP) ? bufferLengthP : unsentTcpLayerBytes;
    //FIXME copy data to bufferP from payloadQueueM at streamPos [dequedBytesM..dequedBytesM+bytes-1]
    return bytes;
}

void TcpLwipDataStreamSendQueue::dequeueTcpLayerMsg(unsigned int msgLengthP)
{
    ASSERT(dequedBytesM + msgLengthP <= enquedBytesM);

    dequedBytesM += msgLengthP;
}

ulong TcpLwipDataStreamSendQueue::getBytesAvailable()
{
    return enquedBytesM - dequedBytesM; // TODO
}

TCPSegment* TcpLwipDataStreamSendQueue::createSegmentWithBytes(
        const void* tcpDataP, unsigned int tcpLengthP)
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

void TcpLwipDataStreamSendQueue::discardAckedBytes(unsigned long bytesP)
{
    // nothing to do here
}

TcpLwipDataStreamReceiveQueue::TcpLwipDataStreamReceiveQueue()
    :
    bytesInQueueM(0)
{
}

TcpLwipDataStreamReceiveQueue::~TcpLwipDataStreamReceiveQueue()
{
    // nothing to do here
}

void TcpLwipDataStreamReceiveQueue::setConnection(TcpLwipConnection *connP)
{
    ASSERT(connP);

    bytesInQueueM = 0;
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
    bytesInQueueM += dataLengthP;
}

unsigned long TcpLwipDataStreamReceiveQueue::getExtractableBytesUpTo()
{
    return bytesInQueueM;
}

TCPDataMsg* TcpLwipDataStreamReceiveQueue::extractBytesUpTo(unsigned long maxBytesP)
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

uint32 TcpLwipDataStreamReceiveQueue::getAmountOfBufferedBytes()
{
    return bytesInQueueM;
}

uint32 TcpLwipDataStreamReceiveQueue::getQueueLength()
{
    return bytesInQueueM ? 1 : 0;
}

void TcpLwipDataStreamReceiveQueue::getQueueStatus()
{
    // TODO
}

void TcpLwipDataStreamReceiveQueue::notifyAboutSending(const TCPSegment *tcpsegP)
{
    // nothing to do
}
