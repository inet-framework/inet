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

#ifndef __INET_TcpLwipVIRTUALDATAQUEUES_H
#define __INET_TcpLwipVIRTUALDATAQUEUES_H

#include <omnetpp.h>

#include "TcpLwipQueues.h"

#include "ByteArrayList.h"
#include "TCPConnection.h"

/**
 * Send/Receive queue that manages "data stream", that is, valid bytes.
 */

class INET_API TcpLwipDataStreamSendQueue : public TcpLwipSendQueue
{
  public:
    /**
     * Ctor.
     */
    TcpLwipDataStreamSendQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TcpLwipDataStreamSendQueue();

    virtual void setConnection(TcpLwipConnection *connP);

    virtual void enqueueAppData(cPacket *msgP);

    virtual unsigned int getBytesForTcpLayer(void* bufferP, unsigned int bufferLengthP);

    virtual void dequeueTcpLayerMsg(unsigned int msgLengthP);

    unsigned  long getBytesAvailable();

    virtual TCPSegment * createSegmentWithBytes(const void* tcpDataP, unsigned int tcpLengthP);

    /**
     * Tells the queue that bytes transmitted and ACKed, so they can be removed from the queue.
     */
    virtual void discardAckedBytes(unsigned long bytesP);

  protected:
    ByteArrayList byteArrayListM;
};

class INET_API TcpLwipDataStreamReceiveQueue : public TcpLwipReceiveQueue
{
  public:
    /**
     * Ctor.
     */
    TcpLwipDataStreamReceiveQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TcpLwipDataStreamReceiveQueue();

    // see TcpLwipReceiveQueue
    virtual void setConnection(TcpLwipConnection *connP);

    // see TcpLwipReceiveQueue
    virtual void insertBytesFromSegment(TCPSegment *tcpsegP, uint32 seqNo, void* bufferP, size_t bufferLengthP);

    /**
     * The method called when data received from NSC
     * The method should set status of the data in queue to received
     * called after socket->read_data() successfull
     */
    virtual void enqueueTcpLayerData(void* dataP, unsigned int dataLengthP);

    // see TcpLwipReceiveQueue
    virtual unsigned long getExtractableBytesUpTo();

    // see TcpLwipReceiveQueue
    virtual TCPDataMsg* extractBytesUpTo(unsigned long maxBytesP);

    /**
     * Returns the number of bytes (out-of-order-segments) currently buffered in queue.
     */
    virtual uint32 getAmountOfBufferedBytes();

    /**
     * Returns the number of blocks currently buffered in queue.
     */
    virtual uint32 getQueueLength();

    /**
     * Shows current queue status.
     */
    virtual void getQueueStatus();

    /**
     * notify the queue about output messages
     *
     * called when connM send out a packet.
     * for read AckNo, if have
     */
    virtual void notifyAboutSending(const TCPSegment *tcpsegP);

  protected:
    ByteArrayList byteArrayListM;
};

#endif
