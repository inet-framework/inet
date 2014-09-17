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

#ifndef __INET_TCPLWIPVIRTUALDATAQUEUES_H
#define __INET_TCPLWIPVIRTUALDATAQUEUES_H

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/tcp_lwip/queues/TcpLwipQueues.h"

namespace inet {

namespace tcp {

/**
 * Send queue that manages "virtual bytes", that is, byte counts only.
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

    virtual unsigned int getBytesForTcpLayer(void *bufferP, unsigned int bufferLengthP) const;

    virtual void dequeueTcpLayerMsg(unsigned int msgLengthP);

    virtual unsigned long getBytesAvailable() const;

    virtual TCPSegment *createSegmentWithBytes(const void *tcpDataP, unsigned int tcpLengthP);

    virtual void discardUpTo(uint32 seqNumP);

  protected:
    long int unsentTcpLayerBytesM;
};

/**
 * Receive queue that manages "virtual bytes", that is, byte counts only.
 */
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
    virtual void notifyAboutIncomingSegmentProcessing(TCPSegment *tcpsegP, uint32 seqNo,
            const void *bufferP, size_t bufferLengthP);

    // see TcpLwipReceiveQueue
    virtual void enqueueTcpLayerData(void *dataP, unsigned int dataLengthP);

    // see TcpLwipReceiveQueue
    virtual cPacket *extractBytesUpTo();

    // see TcpLwipReceiveQueue
    virtual uint32 getAmountOfBufferedBytes() const;

    // see TcpLwipReceiveQueue
    virtual uint32 getQueueLength() const;

    // see TcpLwipReceiveQueue
    virtual void getQueueStatus() const;

    // see TcpLwipReceiveQueue
    virtual void notifyAboutSending(const TCPSegment *tcpsegP);

  protected:
    long int bytesInQueueM;
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCPLWIPVIRTUALDATAQUEUES_H

