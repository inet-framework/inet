#ifndef __TEST__TCPQUEUETESTER_FUNCTIONS

#include "TCPMsgBasedRcvQueue.h"
#include "TCPMsgBasedSendQueue.h"
#include "TCPVirtualDataRcvQueue.h"

// TCPMsgBasedSendQueue:

void enqueue(TCPMsgBasedSendQueue *sq, const char *msgname, ulong numBytes);
void tryenqueue(TCPMsgBasedSendQueue *sq, const char *msgname, ulong numBytes);
TCPSegment *createSegmentWithBytes(TCPMsgBasedSendQueue *sq, uint32 fromSeq, uint32 toSeq);
void discardUpTo(TCPMsgBasedSendQueue *sq, uint32 seqNum);

//////////////////////////////////////////////////////////////

// TCPMsgBasedRcvQueue:

void insertSegment(TCPMsgBasedRcvQueue *rq, TCPSegment *tcpseg);
void tryinsertSegment(TCPMsgBasedRcvQueue *rq, TCPSegment *tcpseg);
void extractBytesUpTo(TCPMsgBasedRcvQueue *rq, uint32 seq);

/////////////////////////////////////////////////////////////////////////

// TCPVirtualDataRcvQueue:

void insertSegment(TCPVirtualDataRcvQueue *q, uint32 beg, uint32 end);
void tryinsertSegment(TCPVirtualDataRcvQueue *q, uint32 beg, uint32 end);
void extractBytesUpTo(TCPVirtualDataRcvQueue *q, uint32 seq);

/////////////////////////////////////////////////////////////////////////

#endif // __TEST__TCPQUEUETESTER_FUNCTIONS
