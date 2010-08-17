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

#include "TcpLwipMsgBasedQueues.h"

#include "TCPCommand.h"
#include "TcpLwipConnection.h"
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
    payload.beginStreamOffset = this->enquedBytesM;
    payload.msg = msgP;
    payloadQueueM.push_back(payload);

    enquedBytesM += bytes;
    unsentTcpLayerBytesM += bytes;
}

int TcpLwipMsgBasedSendQueue::getBytesForTcpLayer(void* bufferP, int bufferLengthP)
{
    ASSERT(bufferP);

    return (unsentTcpLayerBytesM > bufferLengthP) ? bufferLengthP : unsentTcpLayerBytesM;
}

void TcpLwipMsgBasedSendQueue::dequeueTcpLayerMsg(int msgLengthP)
{
    ASSERT(msgLengthP <= unsentTcpLayerBytesM);

    unsentTcpLayerBytesM -= msgLengthP;
}

ulong TcpLwipMsgBasedSendQueue::getBytesAvailable()
{
    return unsentTcpLayerBytesM;
}

TCPSegment* TcpLwipMsgBasedSendQueue::createSegmentWithBytes(
        const void* tcpDataP, int tcpLengthP)
{
    ASSERT(tcpDataP);

    PayloadQueue::iterator i;

    TCPSegment *tcpseg = new TCPSegment("tcp-segment");

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
            tcpseg->addPayloadMessage(msg, i->beginStreamOffset);
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
    bytesInQueueM(0)
{
//    isValidSeqNoM = false;
    lastExtractedBytesM = lastExtractedPayloadBytesM = 0;
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
//    isValidSeqNoM = false;
    lastExtractedBytesM = lastExtractedPayloadBytesM = 0;
    isPayloadExtractAtFirstM = connP->isSendingObjectUpAtFirstByteEnabled();
}

void TcpLwipMsgBasedReceiveQueue::insertBytesFromSegment(
        TCPSegment *tcpsegP, uint32 seqNoP, void* bufferP, size_t bufferLengthP)
{
    ASSERT(tcpsegP);
    ASSERT(bufferP);
    ASSERT(seqLE(tcpsegP->getSequenceNo(), seqNoP));
    uint32 lastSeqNo = seqNoP + bufferLengthP;
    ASSERT(seqGE(tcpsegP->getSequenceNo()+tcpsegP->getPayloadLength(), lastSeqNo));

    cPacket *msg;
    uint64 beginOffsNo;
    while ((msg=tcpsegP->removeFirstPayloadMessage(beginOffsNo)) != NULL)
    {
        if (beginOffsNo >= lastExtractedPayloadBytesM) // if (seqLess(seqNoP, beginSeqNo) && seqLE(beginSeqNo, lastSeqNo))  //FIXME
        {
            // insert, avoiding duplicates
            PayloadList::iterator i = payloadListM.find(beginOffsNo);
            if (i != payloadListM.end())
            {
                ASSERT(msg->getByteLength() == i->second->getByteLength());
                delete i->second;
            }
            payloadListM[beginOffsNo] = msg;
        }
        else
        {
            delete msg;
        }
    }
}

void TcpLwipMsgBasedReceiveQueue::enqueueTcpLayerData(void* dataP, int dataLengthP)
{
    bytesInQueueM += dataLengthP;
}

long TcpLwipMsgBasedReceiveQueue::getExtractableBytesUpTo()
{
    return bytesInQueueM;
}

TCPDataMsg* TcpLwipMsgBasedReceiveQueue::extractBytesUpTo(long maxBytesP)
{
    ASSERT(connM);

    TCPDataMsg *dataMsg = NULL;
    cPacket *objMsg = NULL;

    /*
    if(!isValidSeqNoM)
    {
        isValidSeqNoM = true;
        initialSeqNoM = connM->pcbM->rcv_nxt - bytesInQueueM;
        if(connM->pcbM->state >= LwipTcpLayer::CLOSE_WAIT)
            initialSeqNoM--; // received FIN
    }
    */

    if (bytesInQueueM < maxBytesP)
        maxBytesP = bytesInQueueM;
    uint64 firstOffs = lastExtractedBytesM;
    uint64 lastOffs = firstOffs + maxBytesP;

    // remove old messages
    while( (! payloadListM.empty()) && (payloadListM.begin()->first < lastExtractedPayloadBytesM))
    {
        delete payloadListM.begin()->second;
        payloadListM.erase(payloadListM.begin());
    }

    if(maxBytesP)
    {
        dataMsg = new TCPDataMsg("DATA");
        dataMsg->setKind(TCP_I_DATA);

        // pass up payload messages, in sequence number order
        if (! payloadListM.empty())
        {
            uint64 objOffsNo = payloadListM.begin()->first;
            ASSERT(objOffsNo == lastExtractedPayloadBytesM);
            objMsg = payloadListM.begin()->second;
            int64 objDataLength = objMsg->getByteLength();
            if (isPayloadExtractAtFirstM)
            {
                ASSERT(lastExtractedBytesM <= lastExtractedPayloadBytesM);
                if (objOffsNo == firstOffs)
                {
                    objMsg = payloadListM.begin()->second;
                    int64 objDataLength = objMsg->getByteLength();
                    if (objDataLength < maxBytesP)
                        maxBytesP = objDataLength;

                    payloadListM.erase(payloadListM.begin());
                    lastExtractedPayloadBytesM += objDataLength;
                }
                else if (objOffsNo < lastOffs)
                {
                    // do not add payload object to Msg, truncate segment before first byte of payload data
                    maxBytesP = objOffsNo - firstOffs;
                    objMsg = NULL;
                }
                else
                {
                    // do not add payload object to Msg
                    objMsg = NULL;
                }
            }
            else
            {
                ASSERT(lastExtractedBytesM >= lastExtractedPayloadBytesM);
                ASSERT(objOffsNo <= firstOffs);
                uint64 endOffsNo = objOffsNo + objDataLength;
                if (endOffsNo > lastOffs)
                {
                    // do not add payload object to Msg
                    objMsg = NULL;
                }
                else
                {
                    if (endOffsNo < lastOffs)
                    {
                        // truncate segment after last byte of payload data
                        maxBytesP = endOffsNo - firstOffs;
                    }
                    payloadListM.erase(payloadListM.begin());
                    lastExtractedPayloadBytesM += objDataLength;
                }
            }
        }
        dataMsg->setByteLength(maxBytesP);
        lastExtractedBytesM += maxBytesP;
        bytesInQueueM -= maxBytesP;

        dataMsg->setDataObject(objMsg);
        dataMsg->setIsBegin(isPayloadExtractAtFirstM);
        dataMsg->setKind(TCP_I_DATA);
    }
    return dataMsg;
}

uint32 TcpLwipMsgBasedReceiveQueue::getAmountOfBufferedBytes()
{
    return bytesInQueueM;
}

uint32 TcpLwipMsgBasedReceiveQueue::getQueueLength()
{
    return payloadListM.size();
}

void TcpLwipMsgBasedReceiveQueue::getQueueStatus()
{
    // TODO
}

void TcpLwipMsgBasedReceiveQueue::notifyAboutSending(const TCPSegment *tcpsegP)
{
    // nothing to do
}
