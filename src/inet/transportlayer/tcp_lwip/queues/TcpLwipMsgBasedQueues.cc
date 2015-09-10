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

#include "TcpLwipMsgBasedQueues.h"

#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "inet/transportlayer/tcp_lwip/TcpLwipConnection.h"
#include "inet/common/serializer/tcp/TCPSerializer.h"
#include "inet/transportlayer/tcp_lwip/TCP_lwIP.h"

namespace inet {

namespace tcp {

Register_Class(TcpLwipMsgBasedSendQueue);

Register_Class(TcpLwipMsgBasedReceiveQueue);

TcpLwipMsgBasedSendQueue::TcpLwipMsgBasedSendQueue()
    :
    beginM(0),
    endM(0),
    unsentTcpLayerBytesM(0)
{
}

TcpLwipMsgBasedSendQueue::~TcpLwipMsgBasedSendQueue()
{
    while (!payloadQueueM.empty()) {
        EV_TRACE << "SendQueue Destructor: Drop msg from " << connM->tcpLwipM.getFullPath()
                 << " Queue: seqno=" << payloadQueueM.front().endSequenceNo
                 << ", length=" << payloadQueueM.front().msg->getByteLength() << endl;
        delete payloadQueueM.front().msg;
        payloadQueueM.pop_front();
    }
}

void TcpLwipMsgBasedSendQueue::setConnection(TcpLwipConnection *connP)
{
    TcpLwipSendQueue::setConnection(connP);
    endM = beginM = 0;
    isValidSeqNoM = false;
    unsentTcpLayerBytesM = 0;
}

void TcpLwipMsgBasedSendQueue::enqueueAppData(cPacket *msgP)
{
    ASSERT(msgP);

    uint32 bytes = msgP->getByteLength();
    endM += bytes;
    unsentTcpLayerBytesM += bytes;

    Payload payload;
    payload.endSequenceNo = endM;
    payload.msg = msgP;
    payloadQueueM.push_back(payload);
}

unsigned int TcpLwipMsgBasedSendQueue::getBytesForTcpLayer(void *bufferP, unsigned int bufferLengthP) const
{
    ASSERT(bufferP);

    return (unsentTcpLayerBytesM > bufferLengthP) ? bufferLengthP : unsentTcpLayerBytesM;
}

void TcpLwipMsgBasedSendQueue::dequeueTcpLayerMsg(unsigned int msgLengthP)
{
    ASSERT(msgLengthP <= unsentTcpLayerBytesM);

    unsentTcpLayerBytesM -= msgLengthP;
}

unsigned long TcpLwipMsgBasedSendQueue::getBytesAvailable() const
{
    return unsentTcpLayerBytesM;
}

TCPSegment *TcpLwipMsgBasedSendQueue::createSegmentWithBytes(const void *tcpDataP, unsigned int tcpLengthP)
{
    ASSERT(tcpDataP);

    TCPSegment *tcpseg = serializer::TCPSerializer().deserialize((const unsigned char *)tcpDataP, tcpLengthP, false);

    uint32 fromSeq = tcpseg->getSequenceNo();
    uint32 numBytes = tcpseg->getPayloadLength();
    uint32 toSeq = fromSeq + numBytes;

    if ((!isValidSeqNoM) && (numBytes > 0)) {
        for (auto i = payloadQueueM.begin(); i != payloadQueueM.end(); ++i) {
            i->endSequenceNo += fromSeq;
        }

        beginM += fromSeq;
        endM += fromSeq;
        isValidSeqNoM = true;
    }

    if (numBytes && !seqLE(toSeq, endM))
        throw cRuntimeError("Implementation bug");

    EV_DEBUG << "sendQueue: " << connM->connIdM << ": [" << fromSeq << ":" << toSeq
             << ",l=" << numBytes << "] (unsent bytes:" << unsentTcpLayerBytesM << "\n";

    for (auto i = payloadQueueM.begin(); i != payloadQueueM.end(); ++i) {
        EV_DEBUG << "  buffered msg: endseq=" << i->endSequenceNo
                 << ", length=" << i->msg->getByteLength() << endl;
    }

    const char *payloadName = nullptr;

    if (numBytes > 0) {
        // add payload messages whose endSequenceNo is between fromSeq and fromSeq+numBytes
        auto i = payloadQueueM.begin();

        while (i != payloadQueueM.end() && seqLE(i->endSequenceNo, fromSeq))
            ++i;

        while (i != payloadQueueM.end() && seqLE(i->endSequenceNo, toSeq)) {
            if (!payloadName)
                payloadName = i->msg->getName();

            cPacket *msg = i->msg->dup();
            tcpseg->addPayloadMessage(msg, i->endSequenceNo);
            ++i;
        }
    }

    // give segment a name
    char msgname[80];
    sprintf(msgname, "%.10s%s%s%s(l=%lu,%dmsg)",
            (payloadName ? payloadName : "tcpseg"),
            tcpseg->getSynBit() ? " SYN" : "",
            tcpseg->getFinBit() ? " FIN" : "",
            (tcpseg->getAckBit() && 0 == numBytes) ? " ACK" : "",
            (unsigned long)numBytes,
            tcpseg->getPayloadArraySize());
    tcpseg->setName(msgname);

    discardAckedBytes();

    return tcpseg;
}

void TcpLwipMsgBasedSendQueue::discardAckedBytes()
{
    if (isValidSeqNoM) {
        uint32 seqNum = connM->pcbM->lastack;

        if (seqLE(beginM, seqNum) && seqLE(seqNum, endM)) {
            beginM = seqNum;

            // remove payload messages whose endSequenceNo is below seqNum
            while (!payloadQueueM.empty() && seqLE(payloadQueueM.front().endSequenceNo, seqNum)) {
                delete payloadQueueM.front().msg;
                payloadQueueM.pop_front();
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////

TcpLwipMsgBasedReceiveQueue::TcpLwipMsgBasedReceiveQueue()
    :
    bytesInQueueM(0)
{
}

TcpLwipMsgBasedReceiveQueue::~TcpLwipMsgBasedReceiveQueue()
{
    while (!payloadListM.empty()) {
        delete payloadListM.front().packet;
        payloadListM.pop_front();
    }
}

void TcpLwipMsgBasedReceiveQueue::setConnection(TcpLwipConnection *connP)
{
    ASSERT(connP);

    bytesInQueueM = 0;
    TcpLwipReceiveQueue::setConnection(connP);
    isValidSeqNoM = false;
    lastExtractedSeqNoM = 0;

    while (!payloadListM.empty()) {
        delete payloadListM.front().packet;
        payloadListM.pop_front();
    }
}

void TcpLwipMsgBasedReceiveQueue::notifyAboutIncomingSegmentProcessing(TCPSegment *tcpsegP, uint32 seqNoP, const void *bufferP, size_t bufferLengthP)
{
    ASSERT(tcpsegP);
    ASSERT(bufferP);
    ASSERT(seqLE(tcpsegP->getSequenceNo(), seqNoP));
    uint32 lastSeqNo = seqNoP + bufferLengthP;
    ASSERT(seqGE(tcpsegP->getSequenceNo() + tcpsegP->getPayloadLength(), lastSeqNo));

    cPacket *msg;
    uint32 endSeqNo;

    auto i = payloadListM.begin();
    while ((msg = tcpsegP->removeFirstPayloadMessage(endSeqNo)) != nullptr) {
        if (seqLess(seqNoP, endSeqNo) && seqLE(endSeqNo, lastSeqNo)
            && (!isValidSeqNoM || seqLess(lastExtractedSeqNoM, endSeqNo)))
        {
            while (i != payloadListM.end() && seqLess(i->seqNo, endSeqNo))
                ++i;

            // insert, avoiding duplicates
            if (i != payloadListM.end() && i->seqNo == endSeqNo) {
                ASSERT(msg->getByteLength() == i->packet->getByteLength());
                delete msg;
            }
            else {
                i = payloadListM.insert(i, PayloadItem(endSeqNo, msg));
                ASSERT(seqLE(payloadListM.front().seqNo, payloadListM.back().seqNo));
            }
        }
        else {
            delete msg;
        }
    }
}

void TcpLwipMsgBasedReceiveQueue::enqueueTcpLayerData(void *dataP, unsigned int dataLengthP)
{
    bytesInQueueM += dataLengthP;
}

cPacket *TcpLwipMsgBasedReceiveQueue::extractBytesUpTo()
{
    ASSERT(connM);

    cPacket *dataMsg = nullptr;

    if (!isValidSeqNoM) {
        isValidSeqNoM = true;
        lastExtractedSeqNoM = connM->pcbM->rcv_nxt - bytesInQueueM;

        if (connM->pcbM->state >= LwipTcpLayer::CLOSE_WAIT)
            lastExtractedSeqNoM--; // received FIN
    }

    uint32 firstSeqNo = lastExtractedSeqNoM;
    uint32 lastSeqNo = firstSeqNo + bytesInQueueM;

    // remove old messages
    while ((!payloadListM.empty()) && seqLE(payloadListM.front().seqNo, firstSeqNo)) {
        EV_DEBUG << "Remove old payload MSG: seqno=" << payloadListM.front().seqNo
                 << ", len=" << payloadListM.front().packet->getByteLength() << endl;
        delete payloadListM.front().packet;
        payloadListM.erase(payloadListM.begin());
    }

    // pass up payload messages, in sequence number order
    if (!payloadListM.empty()) {
        uint32 endSeqNo = payloadListM.front().seqNo;

        if (seqLE(endSeqNo, lastSeqNo)) {
            dataMsg = payloadListM.front().packet;
            uint32 dataLength = dataMsg->getByteLength();

            ASSERT(endSeqNo - dataLength == firstSeqNo);

            payloadListM.erase(payloadListM.begin());
            lastExtractedSeqNoM += dataLength;
            bytesInQueueM -= dataLength;

            dataMsg->setKind(TCP_I_DATA);
        }
    }

    return dataMsg;
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

} // namespace tcp

} // namespace inet

