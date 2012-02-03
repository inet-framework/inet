#include "TCPQueueTesterFunctions.h"

void enqueue(TCPMsgBasedSendQueue *sq, const char *msgname, ulong numBytes)
{
    ev << "SQ:" << "enqueue(\"" << msgname << "\", " << numBytes << "):";

    cPacket *msg = new cPacket(msgname);
    msg->setByteLength(numBytes);
    sq->enqueueAppData(msg);

    ev << " --> " << sq->info() <<"\n";
}

void tryenqueue(TCPMsgBasedSendQueue *sq, const char *msgname, ulong numBytes)
{
    ev << "SQ:" << "enqueue(\"" << msgname << "\", " << numBytes << "):";

    cPacket *msg = new cPacket(msgname);
    msg->setByteLength(numBytes);
    try {
    sq->enqueueAppData(msg);
    } catch (cRuntimeError& e) {
        delete msg;
        ev << " --> Error: " << e.what();
    }

    ev << " --> " << sq->info() <<"\n";
}

TCPSegment *createSegmentWithBytes(TCPMsgBasedSendQueue *sq, uint32 fromSeq, uint32 toSeq)
{
    ev << "SQ:" << "createSegmentWithBytes(" << fromSeq << ", " << toSeq << "):";

    ulong numBytes = toSeq-fromSeq;
    TCPSegment *tcpseg = sq->createSegmentWithBytes(fromSeq, numBytes);

    for (int i=0; i<tcpseg->getPayloadArraySize(); i++)
    {
        TCPPayloadMessage& payload = tcpseg->getPayload(i);
        uint32_t startSeq = payload.endSequenceNo - payload.msg->getByteLength();
        ev << (i?", ":" ") << payload.msg->getName() << '[' << startSeq << ".." << payload.endSequenceNo <<')';
    }
    ev << "\n";

    return tcpseg;
}

void discardUpTo(TCPMsgBasedSendQueue *sq, uint32 seqNum)
{
    ev << "SQ:" << "discardUpTo(" << seqNum << "): ";
    sq->discardUpTo(seqNum);

    ev << sq->info() <<"\n";
}

//////////////////////////////////////////////////////////////

void insertSegment(TCPMsgBasedRcvQueue *rq, TCPSegment *tcpseg)
{
    uint32_t beg = tcpseg->getSequenceNo();
    uint32_t end = beg + tcpseg->getPayloadLength();
    ev << "RQ:" << "insertSeg [" << beg << ".." << end << ")";
    uint32 rcv_nxt = rq->insertBytesFromSegment(tcpseg);
    delete tcpseg;
    ev << " --> " << rq->info() <<"\n";
}

void tryinsertSegment(TCPMsgBasedRcvQueue *rq, TCPSegment *tcpseg)
{
    uint32_t beg = tcpseg->getSequenceNo();
    uint32_t end = beg + tcpseg->getPayloadLength();
    ev << "RQ:" << "insertSeg [" << beg << ".." << end << ")";
    try {
        uint32 rcv_nxt = rq->insertBytesFromSegment(tcpseg);
    } catch (cRuntimeError& e) {
        ev << " --> Error: " << e.what();
    }
    delete tcpseg;

    ev << " --> " << rq->info() <<"\n";
}

void extractBytesUpTo(TCPMsgBasedRcvQueue *rq, uint32 seq)
{
    ev << "RQ:" << "extractUpTo(" << seq << "): <";
    cPacket *msg;
    while ((msg=rq->extractBytesUpTo(seq)) != NULL)
    {
        ev << " < " << msg->getName() << ": " << msg->getByteLength() << " bytes >";
        delete msg;
    }
    ev << " > --> " << rq->info() <<"\n";
}

/////////////////////////////////////////////////////////////////////////

void insertSegment(TCPVirtualDataRcvQueue *q, uint32 beg, uint32 end)
{
    ev << "RQ:" << "insertSeg [" << beg << ".." << end << ")";

    TCPSegment *tcpseg = new TCPSegment();
    tcpseg->setSequenceNo(beg);
    tcpseg->setPayloadLength(end-beg);
    uint32 rcv_nxt = q->insertBytesFromSegment(tcpseg);
    delete tcpseg;

    ev << " --> " << q->info() <<"\n";
}

void tryinsertSegment(TCPVirtualDataRcvQueue *q, uint32 beg, uint32 end)
{
    ev << "RQ:" << "insertSeg [" << beg << ".." << end << ")";
    TCPSegment *tcpseg = new TCPSegment();
    tcpseg->setSequenceNo(beg);
    tcpseg->setPayloadLength(end-beg);
    try {
        uint32 rcv_nxt = q->insertBytesFromSegment(tcpseg);
    } catch (cRuntimeError& e) {
        ev << " --> Error: " << e.what();
    }
    delete tcpseg;
    ev << " --> " << q->info() <<"\n";
}

void extractBytesUpTo(TCPVirtualDataRcvQueue *q, uint32 seq)
{
    ev << "RQ:" << "extractUpTo(" << seq << "):";
    cPacket *msg;
    while ((msg=q->extractBytesUpTo(seq))!=NULL)
    {
        ev << " msglen=" << msg->getByteLength();
        delete msg;
    }
    ev << " --> " << q->info() <<"\n";
}

