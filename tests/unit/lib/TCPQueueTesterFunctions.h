#ifndef __TEST__TCPQUEUETESTER_FUNCTIONS

#include "inet/transportlayer/tcp_common/TCPSegment.h"
#include "inet/transportlayer/tcp/queues/TCPMsgBasedRcvQueue.h"
#include "inet/transportlayer/tcp/queues/TCPMsgBasedSendQueue.h"
#include "inet/transportlayer/tcp/queues/TCPVirtualDataRcvQueue.h"


// TCPMsgBasedSendQueue:

void enqueue(::inet::tcp::TCPMsgBasedSendQueue *sq, const char *msgname, ulong numBytes);
void tryenqueue(::inet::tcp::TCPMsgBasedSendQueue *sq, const char *msgname, ulong numBytes);
::inet::tcp::TCPSegment *createSegmentWithBytes(::inet::tcp::TCPMsgBasedSendQueue *sq, uint32 fromSeq, uint32 toSeq);
void discardUpTo(::inet::tcp::TCPMsgBasedSendQueue *sq, uint32 seqNum);

//////////////////////////////////////////////////////////////

// TCPMsgBasedRcvQueue:

void insertSegment(::inet::tcp::TCPMsgBasedRcvQueue *rq, ::inet::tcp::TCPSegment *tcpseg);
void tryinsertSegment(::inet::tcp::TCPMsgBasedRcvQueue *rq, ::inet::tcp::TCPSegment *tcpseg);
void extractBytesUpTo(::inet::tcp::TCPMsgBasedRcvQueue *rq, uint32 seq);

/////////////////////////////////////////////////////////////////////////

// TCPVirtualDataRcvQueue:

void insertSegment(::inet::tcp::TCPVirtualDataRcvQueue *q, uint32 beg, uint32 end);
void tryinsertSegment(::inet::tcp::TCPVirtualDataRcvQueue *q, uint32 beg, uint32 end);
void extractBytesUpTo(::inet::tcp::TCPVirtualDataRcvQueue *q, uint32 seq);

/////////////////////////////////////////////////////////////////////////

#endif // __TEST__TCPQUEUETESTER_FUNCTIONS
