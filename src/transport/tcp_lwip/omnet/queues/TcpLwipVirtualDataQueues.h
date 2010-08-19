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

#include "TCPConnection.h"

/**
 * Send/Receive queue that manages "virtual bytes", that is, byte counts only.
 */

class INET_API TcpLwipVirtualDataSendQueue : public TcpLwipSendQueue
{
  public:
    /**
     * Ctor.
     */
    TcpLwipVirtualDataSendQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TcpLwipVirtualDataSendQueue();

    virtual void setConnection(TcpLwipConnection *connP);

    virtual void enqueueAppData(cPacket *msgP);

    virtual unsigned int getBytesForTcpLayer(void* bufferP, unsigned int bufferLengthP);

    virtual void dequeueTcpLayerMsg(unsigned int msgLengthP);

    ulong getBytesAvailable();

    virtual TCPSegment * createSegmentWithBytes(const void* tcpDataP, unsigned int tcpLengthP);

    virtual void discardAckedBytes(unsigned long bytesP);

  protected:
    unsigned long unsentTcpLayerBytesM;
};

class INET_API TcpLwipVirtualDataReceiveQueue : public TcpLwipReceiveQueue
{
  public:
    /**
     * Ctor.
     */
    TcpLwipVirtualDataReceiveQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TcpLwipVirtualDataReceiveQueue();

    // see TcpLwipReceiveQueue
    virtual void setConnection(TcpLwipConnection *connP);

    // see TcpLwipReceiveQueue
    virtual void insertBytesFromSegment(TCPSegment *tcpsegP, uint32 seqNo, void* bufferP, size_t bufferLengthP);

    // see TcpLwipReceiveQueue
    virtual void enqueueTcpLayerData(void* dataP, unsigned int dataLengthP);

    // see TcpLwipReceiveQueue
    virtual unsigned long getExtractableBytesUpTo();

    // see TcpLwipReceiveQueue
    virtual TCPDataMsg* extractBytesUpTo(unsigned long maxBytesP);

    // see TcpLwipReceiveQueue
    virtual uint32 getAmountOfBufferedBytes();

    // see TcpLwipReceiveQueue
    virtual uint32 getQueueLength();

    // see TcpLwipReceiveQueue
    virtual void getQueueStatus();

    // see TcpLwipReceiveQueue
    virtual void notifyAboutSending(const TCPSegment *tcpsegP);

  protected:
    unsigned long bytesInQueueM;
};

#endif
