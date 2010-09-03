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


#include <omnetpp.h>

#include "TcpLwipMsgBasedQueues.h"

#include "TCPCommand.h"
#include "TcpLwipConnection.h"
#include "TCPSegmentWithData.h"
#include "TCPSerializer.h"
#include "TCP_lwip.h"


Register_Class(TcpLwipMsgBasedSendQueue);
Register_Class(TcpLwipMsgBasedReceiveQueue);


TcpLwipMsgBasedSendQueue::TcpLwipMsgBasedSendQueue()
    :
    initialSeqNoM(0),
    enquedBytesM(0),
    ackedBytesM(0),
    isValidSeqNoM(false),
    unsentTcpLayerBytesM(0)
{
}

TcpLwipMsgBasedSendQueue::~TcpLwipMsgBasedSendQueue()
{
    while (! payloadQueueM.empty())
    {
        EV << "SendQueue Destructor: Drop msg from " << connM->tcpLwipM.getFullPath() <<
                " Queue: offset=" << payloadQueueM.front().beginStreamOffset <<
                ", length=" << payloadQueueM.front().msg->getByteLength() << endl;
        delete payloadQueueM.front().msg;
        payloadQueueM.pop_front();
    }
}

void TcpLwipMsgBasedSendQueue::setConnection(TcpLwipConnection *connP)
{
    TcpLwipSendQueue::setConnection(connP);
    initialSeqNoM = 0;
    enquedBytesM = 0;
    ackedBytesM = 0;
    isValidSeqNoM =false;
    unsentTcpLayerBytesM = 0;
}

void TcpLwipMsgBasedSendQueue::enqueueAppData(cPacket *msgP)
{
    ASSERT(msgP);

    uint32 bytes = msgP->getByteLength();

    Payload payload;
    payload.beginStreamOffset = enquedBytesM;
    payload.msg = msgP;
    payloadQueueM.push_back(payload);

    enquedBytesM += bytes;
    unsentTcpLayerBytesM += bytes;
}

unsigned int TcpLwipMsgBasedSendQueue::getBytesForTcpLayer(void* bufferP, unsigned int bufferLengthP) const
{
    ASSERT(bufferP);

    return (unsentTcpLayerBytesM > bufferLengthP) ? bufferLengthP : unsentTcpLayerBytesM;
}

void TcpLwipMsgBasedSendQueue::dequeueTcpLayerMsg(unsigned int msgLengthP)
{
    ASSERT(msgLengthP <= unsentTcpLayerBytesM);

    unsentTcpLayerBytesM -= msgLengthP;
}

ulong TcpLwipMsgBasedSendQueue::getBytesAvailable() const
{
    return unsentTcpLayerBytesM;
}

TCPSegment* TcpLwipMsgBasedSendQueue::createSegmentWithBytes(
        const void* tcpDataP, unsigned int tcpLengthP)
{
    ASSERT(tcpDataP);

    PayloadQueue::iterator i;

    TCPSegmentWithMessages *tcpseg = new TCPSegmentWithMessages("tcp-segment");

    TCPSerializer().parse((const unsigned char *)tcpDataP, tcpLengthP, tcpseg);

    uint32 fromSeq = tcpseg->getSequenceNo();
    uint32 numBytes = tcpseg->getPayloadLength();
    if( (! isValidSeqNoM) && (numBytes > 0))
    {
        initialSeqNoM = fromSeq;
        isValidSeqNoM = true;
    }
    uint32 ackedSeqNo = initialSeqNoM+ackedBytesM;
    uint64 fromOffs = ackedBytesM + (uint32)(fromSeq-ackedSeqNo);
    uint64 toOffs = fromOffs + numBytes;

    EV << "sendQueue: " << connM->connIdM << ": [" << fromOffs << ":" << toOffs << ",l=" << numBytes << "] (unsent bytes:" << unsentTcpLayerBytesM << "\n";

#ifdef DEBUG_LWIP
    for(i=payloadQueueM.begin(); i!=payloadQueueM.end(); ++i)
    {
        EV << "  buffered msg: beginseq=" << i->beginStreamOffset << ", length=" << i->msg->getByteLength() << endl;
    }
#endif

    const char *payloadName = NULL;
    if (numBytes > 0)
    {
        // add payload messages whose beginSequenceNo is between fromSeq and fromSeq+numBytes
        i = payloadQueueM.begin();
        while (i!=payloadQueueM.end() && (i->beginStreamOffset < fromOffs))
            ++i;
        while (i!=payloadQueueM.end() && (i->beginStreamOffset < toOffs))
        {
            if (!payloadName)
                payloadName = i->msg->getName();
            cPacket* msg = i->msg->dup();
            tcpseg->addPayloadMessage(msg, i->beginStreamOffset, i->beginStreamOffset - fromOffs);
            ++i;
        }
    }

    // give segment a name
    char msgname[80];
    sprintf(msgname, "%.10s%s%s%s(l=%lu,%dmsg)",
            (payloadName ? payloadName : "tcpseg"),
            tcpseg->getSynBit() ? " SYN":"",
            tcpseg->getFinBit() ? " FIN":"",
            (tcpseg->getAckBit() && 0==numBytes) ? " ACK":"",
            (unsigned long)numBytes,
            tcpseg->getPayloadArraySize());
    tcpseg->setName(msgname);

    return tcpseg;
}

void TcpLwipMsgBasedSendQueue::discardAckedBytes(unsigned long bytesP)
{
    ASSERT(isValidSeqNoM);

    ackedBytesM += bytesP;

    ASSERT((uint32)(initialSeqNoM + ackedBytesM) == connM->pcbM->lastack);

    // remove payload messages whose beginSequenceNo is below seqNum
    while (!payloadQueueM.empty() && payloadQueueM.front().beginStreamOffset < ackedBytesM)
    {
        delete payloadQueueM.front().msg;
        payloadQueueM.pop_front();
    }
}

////////////////////////////////////////////////////////////////////////

TcpLwipMsgBasedReceiveQueue::TcpLwipMsgBasedReceiveQueue()
    :
    isPayloadExtractAtFirstM(333),
    lastExtractedBytesM(0),
    lastExtractedPayloadBytesM(0),
    bytesInQueueM(0)
{
}

TcpLwipMsgBasedReceiveQueue::~TcpLwipMsgBasedReceiveQueue()
{
    PayloadList::iterator i;
    while ((i = payloadListM.begin()) != payloadListM.end())
    {
        delete i->second;
        payloadListM.erase(i);
    }
}

void TcpLwipMsgBasedReceiveQueue::setConnection(TcpLwipConnection *connP)
{
    ASSERT(connP);

    bytesInQueueM = 0;
    TcpLwipReceiveQueue::setConnection(connP);
    lastExtractedBytesM = lastExtractedPayloadBytesM = 0;
    isPayloadExtractAtFirstM = connP->isSendingObjectUpAtFirstByteEnabled();
}

void TcpLwipMsgBasedReceiveQueue::notifyAboutIncomingSegmentProcessing(
        TCPSegment *tcpsegP, uint32 seqNoP, const void* bufferP, size_t bufferLengthP)
{
    ASSERT(tcpsegP);
    ASSERT(bufferP);
    ASSERT(seqLE(tcpsegP->getSequenceNo(), seqNoP));
    uint32 lastSeqNo = seqNoP + bufferLengthP;
    ASSERT(seqGE(tcpsegP->getSequenceNo()+tcpsegP->getPayloadLength(), lastSeqNo));

    if (0==bufferLengthP && 0==tcpsegP->getPayloadLength())
        return;

    TCPSegmentWithMessages *tcpseg = check_and_cast<TCPSegmentWithMessages *>(tcpsegP);

    cPacket *msg;
    uint64 streamOffsNo, segmentOffsNo;
    while ((msg=tcpseg->removeFirstPayloadMessage(streamOffsNo, segmentOffsNo)) != NULL)
    {
        if (streamOffsNo >= lastExtractedPayloadBytesM && segmentOffsNo < tcpseg->getPayloadLength())
        {
            // insert, avoiding duplicates
            PayloadList::iterator i = payloadListM.find(streamOffsNo);
            if (i != payloadListM.end())
            {
                ASSERT(msg->getByteLength() == i->second->getByteLength());
                delete i->second;
            }
            payloadListM[streamOffsNo] = msg;
        }
        else
        {
            delete msg;
        }
    }
}

void TcpLwipMsgBasedReceiveQueue::enqueueTcpLayerData(void* dataP, unsigned int dataLengthP)
{
    bytesInQueueM += dataLengthP;
}

unsigned long TcpLwipMsgBasedReceiveQueue::getExtractableBytesUpTo() const
{
    return bytesInQueueM;
}

TCPDataMsg* TcpLwipMsgBasedReceiveQueue::extractBytesUpTo(unsigned long maxBytesP)
{
    ASSERT(connM);

    TCPDataMsg *msg = NULL;
    cPacket *objMsg = NULL;
    uint64 nextPayloadBegin = lastExtractedPayloadBytesM;
    uint64 nextPayloadLength = 0;
    uint64 nextPayloadOffs = 0;

    if (!payloadListM.empty())
    {
        nextPayloadBegin = payloadListM.begin()->first;
        nextPayloadLength = payloadListM.begin()->second->getByteLength();
    }
    uint64 nextPayloadEnd = nextPayloadBegin + nextPayloadLength;

    ASSERT(nextPayloadBegin == lastExtractedPayloadBytesM);

    if (isPayloadExtractAtFirstM)
    {
        nextPayloadOffs = nextPayloadBegin;
        ASSERT(lastExtractedBytesM <= lastExtractedPayloadBytesM);
        ASSERT(nextPayloadBegin >= lastExtractedBytesM);
        if (nextPayloadBegin == lastExtractedBytesM)
        {
            if (maxBytesP > nextPayloadLength)
                maxBytesP = nextPayloadLength;
        }
        else // nextPayloadBegin > lastExtractedBytesM
        {
            ulong extractableBytes = nextPayloadBegin - lastExtractedBytesM;
            if (maxBytesP > extractableBytes)
                maxBytesP = extractableBytes;
        }
    }
    else
    {
        nextPayloadOffs = nextPayloadEnd -1;
        ulong extractableBytes = nextPayloadEnd - lastExtractedBytesM;
        ASSERT(lastExtractedBytesM >= lastExtractedPayloadBytesM);
        if (maxBytesP > extractableBytes)
            maxBytesP = extractableBytes;
    }

    if (maxBytesP)
    {
        msg = new TCPDataMsg("DATA");
        msg->setKind(TCP_I_DATA);
        msg->setByteLength(maxBytesP);
        if (!payloadListM.empty() && lastExtractedBytesM <= nextPayloadOffs && nextPayloadOffs < lastExtractedBytesM + maxBytesP)
        {
            objMsg = payloadListM.begin()->second;
            payloadListM.erase(payloadListM.begin());
            lastExtractedPayloadBytesM = nextPayloadEnd;
        }
        msg->setPayloadPacket(objMsg);
        msg->setIsPayloadStart(isPayloadExtractAtFirstM);
        lastExtractedBytesM += maxBytesP;
    }
    return msg;
}

uint32 TcpLwipMsgBasedReceiveQueue::getAmountOfBufferedBytes() const
{
    return bytesInQueueM;
}

uint32 TcpLwipMsgBasedReceiveQueue::getQueueLength() const
{
    return payloadListM.size();
}

void TcpLwipMsgBasedReceiveQueue::getQueueStatus() const
{
    // TODO
}

void TcpLwipMsgBasedReceiveQueue::notifyAboutSending(const TCPSegment *tcpsegP)
{
    // nothing to do
}
