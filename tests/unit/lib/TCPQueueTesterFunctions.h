#ifndef __TEST__TCPQUEUETESTER_FUNCTIONS

#include "inet/common/packet/Packet.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"

using namespace inet;
using namespace tcp;

// TcpSendQueue:

void enqueue(TcpSendQueue *sq, const char *msgname, ulong numBytes);
void tryenqueue(TcpSendQueue *sq, const char *msgname, ulong numBytes);
Packet *createSegmentWithBytes(TcpSendQueue *sq, uint32_t fromSeq, uint32_t toSeq);
void discardUpTo(TcpSendQueue *sq, uint32_t seqNum);

//////////////////////////////////////////////////////////////

// TcpReceiveQueue:

void insertSegment(TcpReceiveQueue *rq, Packet *tcpseg);
void tryinsertSegment(TcpReceiveQueue *rq, Packet *tcpseg);
void extractBytesUpTo(TcpReceiveQueue *rq, uint32_t seq);

void insertSegment(TcpReceiveQueue *q, uint32_t beg, uint32_t end);
void tryinsertSegment(TcpReceiveQueue *q, uint32_t beg, uint32_t end);

/////////////////////////////////////////////////////////////////////////

#endif // __TEST__TCPQUEUETESTER_FUNCTIONS
