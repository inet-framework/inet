#include "TCPQueueTesterFunctions.h"

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"

using namespace inet;
using namespace inet::tcp;

void enqueue(TcpSendQueue *sq, const char *msgname, ulong numBytes)
{
    EV << "SQ:" << "enqueue(\"" << msgname << "\", " << numBytes << "):";

    Packet *msg = new Packet(msgname);
    const auto & bytes = makeShared<ByteCountChunk>(B(numBytes));
    msg->insertAtBack(bytes);
    ASSERT(msg->getByteLength() == numBytes);
    sq->enqueueAppData(msg);

    EV << " --> " << sq->str() <<"\n";
}

void tryenqueue(TcpSendQueue *sq, const char *msgname, ulong numBytes)
{
    EV << "SQ:" << "enqueue(\"" << msgname << "\", " << numBytes << "):";

    Packet *msg = new Packet(msgname);
    const auto & bytes = makeShared<ByteCountChunk>(B(numBytes));
    msg->insertAtBack(bytes);
    ASSERT(msg->getByteLength() == numBytes);
    try {
    sq->enqueueAppData(msg);
    } catch (cRuntimeError& e) {
        delete msg;
        EV << " --> Error: " << e.what();
    }

    EV << " --> " << sq->str() <<"\n";
}

Packet *createSegmentWithBytes(TcpSendQueue *sq, uint32 fromSeq, uint32 toSeq)
{
    EV << "SQ:" << "createSegmentWithBytes(" << fromSeq << ", " << toSeq << "):";

    ulong numBytes = toSeq-fromSeq;
    Packet *pk = sq->createSegmentWithBytes(fromSeq, numBytes);

    {
        Packet *tcpseg = pk->dup();
        uint32_t startSeq = fromSeq;
        for (int i = 0; tcpseg->getByteLength() > 0; i++)
        {
            const auto& payload = tcpseg->popAtFront<Chunk>();
            int len = B(payload->getChunkLength()).get();
            EV << (i?", ":" ") << payload->getClassName() << '[' << startSeq << ".." << startSeq + len <<')';
            startSeq += len;
        }
        EV << "\n";
        delete tcpseg;
    }

    const auto& tcpHdr = makeShared<TcpHeader>();
    tcpHdr->setSequenceNo(fromSeq);
    pk->insertAtFront(tcpHdr);

    return pk;
}

void discardUpTo(TcpSendQueue *sq, uint32 seqNum)
{
    EV << "SQ:" << "discardUpTo(" << seqNum << "): ";
    sq->discardUpTo(seqNum);

    EV << sq->str() <<"\n";
}

//////////////////////////////////////////////////////////////

void insertSegment(TcpReceiveQueue *rq, Packet *tcpseg)
{
    const auto& tcphdr = tcpseg->peekAtFront<TcpHeader>();
    uint32_t beg = tcphdr->getSequenceNo();
    uint32_t end = beg + tcpseg->getByteLength() - B(tcphdr->getHeaderLength()).get();
    EV << "RQ:" << "insertSeg [" << beg << ".." << end << ")";
    uint32 rcv_nxt = rq->insertBytesFromSegment(tcpseg, tcphdr);
    delete tcpseg;
    EV << " --> " << rq->str() <<"\n";
}

void tryinsertSegment(TcpReceiveQueue *rq, Packet *tcpseg)
{
    const auto& tcphdr = tcpseg->peekAtFront<TcpHeader>();
    uint32_t beg = tcphdr->getSequenceNo();
    uint32_t end = beg + tcpseg->getByteLength() - B(tcphdr->getHeaderLength()).get();
    EV << "RQ:" << "insertSeg [" << beg << ".." << end << ")";
    try {
        uint32 rcv_nxt = rq->insertBytesFromSegment(tcpseg,tcphdr);
    } catch (cRuntimeError& e) {
        EV << " --> Error: " << e.what();
    }
    delete tcpseg;

    EV << " --> " << rq->str() <<"\n";
}

void extractBytesUpTo(TcpReceiveQueue *rq, uint32 seq)
{
    EV << "RQ:" << "extractUpTo(" << seq << "): <";
    cPacket *msg;
    while ((msg=rq->extractBytesUpTo(seq)) != NULL)
    {
        EV << " < " << msg->getName() << ": " << msg->getByteLength() << " bytes >";
        delete msg;
    }
    EV << " > --> " << rq->str() <<"\n";
}

/////////////////////////////////////////////////////////////////////////

void insertSegment(TcpReceiveQueue *q, uint32 beg, uint32 end)
{
    EV << "RQ:" << "insertSeg [" << beg << ".." << end << ")";

    Packet *msg = new Packet();
    unsigned int numBytes = end - beg;
    const auto& bytes = makeShared<ByteCountChunk>(B(numBytes));
    msg->insertAtBack(bytes);
    const auto& tcpseg = makeShared<TcpHeader>();
    tcpseg->setSequenceNo(beg);
    msg->insertAtFront(tcpseg);

    uint32 rcv_nxt = q->insertBytesFromSegment(msg, tcpseg);
    delete msg;

    EV << " --> " << q->str() <<"\n";
}

void tryinsertSegment(TcpReceiveQueue *q, uint32 beg, uint32 end)
{
    EV << "RQ:" << "insertSeg [" << beg << ".." << end << ")";

    Packet *msg = new Packet();
    unsigned int numBytes = end - beg;
    const auto& bytes = makeShared<ByteCountChunk>(B(numBytes));
    msg->insertAtBack(bytes);
    const auto& tcpseg = makeShared<TcpHeader>();
    tcpseg->setSequenceNo(beg);
    msg->insertAtFront(tcpseg);

    try {
        uint32 rcv_nxt = q->insertBytesFromSegment(msg, tcpseg);
    } catch (cRuntimeError& e) {
        EV << " --> Error: " << e.what();
    }
    delete msg;
    EV << " --> " << q->str() <<"\n";
}

