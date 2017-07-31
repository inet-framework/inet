#ifndef __TEST__TCPQUEUETESTER_FUNCTIONS

#include "inet/common/packet/Packet.h"
#include "inet/transportlayer/tcp/TCPReceiveQueue.h"
#include "inet/transportlayer/tcp/TCPSendQueue.h"

using namespace inet;
using namespace tcp;

// TCPSendQueue:

void enqueue(TCPSendQueue *sq, const char *msgname, ulong numBytes);
void tryenqueue(TCPSendQueue *sq, const char *msgname, ulong numBytes);
Packet *createSegmentWithBytes(TCPSendQueue *sq, uint32 fromSeq, uint32 toSeq);
void discardUpTo(TCPSendQueue *sq, uint32 seqNum);

//////////////////////////////////////////////////////////////

// TCPReceiveQueue:

void insertSegment(TCPReceiveQueue *rq, Packet *tcpseg);
void tryinsertSegment(TCPReceiveQueue *rq, Packet *tcpseg);
void extractBytesUpTo(TCPReceiveQueue *rq, uint32 seq);

void insertSegment(TCPReceiveQueue *q, uint32 beg, uint32 end);
void tryinsertSegment(TCPReceiveQueue *q, uint32 beg, uint32 end);

/////////////////////////////////////////////////////////////////////////

#endif // __TEST__TCPQUEUETESTER_FUNCTIONS
