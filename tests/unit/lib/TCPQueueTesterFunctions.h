#ifndef __TEST__TCPQUEUETESTER_FUNCTIONS

#include "inet/common/packet/Packet.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"

using namespace inet;
using namespace tcp;

// TcpSendQueue:

void enqueue(TcpSendQueue *sq, const char *msgname, ulong numBytes);
void tryenqueue(TcpSendQueue *sq, const char *msgname, ulong numBytes);
Packet *createSegmentWithBytes(TcpSendQueue *sq, uint32 fromSeq, uint32 toSeq);
void discardUpTo(TcpSendQueue *sq, uint32 seqNum);

//////////////////////////////////////////////////////////////

// TcpReceiveQueue:

void insertSegment(TcpReceiveQueue *rq, Packet *tcpseg);
void tryinsertSegment(TcpReceiveQueue *rq, Packet *tcpseg);
void extractBytesUpTo(TcpReceiveQueue *rq, uint32 seq);

void insertSegment(TcpReceiveQueue *q, uint32 beg, uint32 end);
void tryinsertSegment(TcpReceiveQueue *q, uint32 beg, uint32 end);

/////////////////////////////////////////////////////////////////////////

#endif // __TEST__TCPQUEUETESTER_FUNCTIONS
