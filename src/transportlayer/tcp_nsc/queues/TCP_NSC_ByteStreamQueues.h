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

#ifndef __INET_TCP_NSC_DATASTREAMQUEUES_H
#define __INET_TCP_NSC_DATASTREAMQUEUES_H


#include "INETDefs.h"

#include "TCP_NSC_Queues.h"
#include "ByteArrayBuffer.h"


/**
 * Send queue that manages actual bytes.
 */
class INET_API TCP_NSC_ByteStreamSendQueue : public TCP_NSC_SendQueue
{
  public:
    /**
     * Ctor.
     */
    TCP_NSC_ByteStreamSendQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TCP_NSC_ByteStreamSendQueue();

    virtual void setConnection(TCP_NSC_Connection *connP);

    virtual void enqueueAppData(cPacket *msgP);

    virtual int getBytesForTcpLayer(void* bufferP, int bufferLengthP) const;

    virtual void dequeueTcpLayerMsg(int msgLengthP);

    virtual unsigned long getBytesAvailable() const;

    virtual TCPSegment* createSegmentWithBytes(const void *tcpDataP, int tcpLengthP);

    virtual void discardUpTo(uint32 seqNumP);

  protected:
    ByteArrayBuffer byteArrayBufferM;
};


/**
 * Receive queue that manages actual bytes.
 */
class INET_API TCP_NSC_ByteStreamReceiveQueue : public TCP_NSC_ReceiveQueue
{
  public:
    /**
     * Ctor.
     */
    TCP_NSC_ByteStreamReceiveQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TCP_NSC_ByteStreamReceiveQueue();

    virtual void setConnection(TCP_NSC_Connection *connP);

    virtual void notifyAboutIncomingSegmentProcessing(TCPSegment *tcpsegP);

    virtual void enqueueNscData(void* dataP, int dataLengthP);

    virtual cPacket* extractBytesUpTo();

    virtual uint32 getAmountOfBufferedBytes() const;

    virtual uint32 getQueueLength() const;

    virtual void getQueueStatus() const;

    virtual void notifyAboutSending(const TCPSegment *tcpsegP);

  protected:
    ByteArrayBuffer byteArrayBufferM;
};

#endif // __INET_TCP_NSC_DATASTREAMQUEUES_H
