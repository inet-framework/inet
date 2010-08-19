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

#ifndef __INET_TcpLwipMSGBASEDQUEUES_H
#define __INET_TcpLwipMSGBASEDQUEUES_H

#include <omnetpp.h>

#include "TcpLwipQueues.h"

#include "TCPConnection.h"

/**
 * Send/Receive queue that manages messages.
 */

class INET_API TcpLwipMsgBasedSendQueue : public TcpLwipSendQueue
{
  protected:
    struct Payload
    {
        uint64 beginStreamOffset;   // the offset of first byte of packet in the datastream
        cPacket *msg;
    };
    typedef std::list<Payload> PayloadQueue;
    PayloadQueue payloadQueueM;

    uint32 initialSeqNoM;  // the initial sequence number ( sequence no of first byte)
    uint64 enquedBytesM;
    uint64 ackedBytesM;
    bool isValidSeqNoM;
    unsigned long unsentTcpLayerBytesM;

  public:
    /**
     * Ctor.
     */
    TcpLwipMsgBasedSendQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TcpLwipMsgBasedSendQueue();

    virtual void setConnection(TcpLwipConnection *connP);

    virtual void enqueueAppData(cPacket *msgP);

    virtual unsigned int getBytesForTcpLayer(void* bufferP, unsigned int bufferLengthP);

    virtual void dequeueTcpLayerMsg(unsigned int msgLengthP);

    ulong getBytesAvailable();

    virtual TCPSegment * createSegmentWithBytes(const void* tcpDataP, unsigned int tcpLengthP);

    virtual void discardAckedBytes(unsigned long bytesP);
};

class INET_API TcpLwipMsgBasedReceiveQueue : public TcpLwipReceiveQueue
{
  protected:
    typedef std::map<uint64, cPacket*> PayloadList;
    PayloadList payloadListM;
//    bool isValidSeqNoM;
    bool isPayloadExtractAtFirstM;
    uint64 lastExtractedBytesM;
    uint64 lastExtractedPayloadBytesM;

  public:
    /**
     * Ctor.
     */
    TcpLwipMsgBasedReceiveQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TcpLwipMsgBasedReceiveQueue();

    virtual void setConnection(TcpLwipConnection *connP);

    virtual void insertBytesFromSegment(TCPSegment *tcpsegP, uint32 seqNo, void* bufferP, size_t bufferLengthP);

    virtual void enqueueTcpLayerData(void* dataP, unsigned int dataLengthP);

    virtual unsigned long getExtractableBytesUpTo();

    virtual TCPDataMsg* extractBytesUpTo(unsigned long maxBytesP);

    virtual uint32 getAmountOfBufferedBytes();

    virtual uint32 getQueueLength();

    virtual void getQueueStatus();

    virtual void notifyAboutSending(const TCPSegment *tcpsegP);

  protected:
    unsigned long bytesInQueueM;
};

#endif
