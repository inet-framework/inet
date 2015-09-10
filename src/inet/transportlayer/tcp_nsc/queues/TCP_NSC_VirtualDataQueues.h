//
// Copyright (C) 2004 Andras Varga
//               2009 Zoltan Bojthe
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

#ifndef __INET_TCP_NSC_VIRTUALDATAQUEUES_H
#define __INET_TCP_NSC_VIRTUALDATAQUEUES_H

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/tcp_nsc/queues/TCP_NSC_Queues.h"

namespace inet {

namespace tcp {

/**
 * Send queue that manages "virtual bytes", that is, byte counts only.
 */
class INET_API TCP_NSC_VirtualDataSendQueue : public TCP_NSC_SendQueue
{
  public:
    /**
     * Ctor.
     */
    TCP_NSC_VirtualDataSendQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TCP_NSC_VirtualDataSendQueue();

    virtual void setConnection(TCP_NSC_Connection *connP) override;

    virtual void enqueueAppData(cPacket *msgP) override;

    virtual int getBytesForTcpLayer(void *bufferP, int bufferLengthP) const override;

    virtual void dequeueTcpLayerMsg(int msgLengthP) override;

    virtual unsigned long getBytesAvailable() const override;

    virtual TCPSegment *createSegmentWithBytes(const void *tcpDataP, int tcpLengthP) override;

    virtual void discardUpTo(uint32 seqNumP) override;

  protected:
    long int unsentNscBytesM;
};

/**
 * Send/Receive queue that manages "virtual bytes", that is, byte counts only.
 */
class INET_API TCP_NSC_VirtualDataReceiveQueue : public TCP_NSC_ReceiveQueue
{
  public:
    /**
     * Ctor.
     */
    TCP_NSC_VirtualDataReceiveQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TCP_NSC_VirtualDataReceiveQueue();

    virtual void setConnection(TCP_NSC_Connection *connP) override;

    virtual void notifyAboutIncomingSegmentProcessing(TCPSegment *tcpsegP) override;

    virtual void enqueueNscData(void *dataP, int dataLengthP) override;

    virtual cPacket *extractBytesUpTo() override;

    virtual uint32 getAmountOfBufferedBytes() const override;

    virtual uint32 getQueueLength() const override;

    virtual void getQueueStatus() const override;

    virtual void notifyAboutSending(const TCPSegment *tcpsegP) override;

  protected:
    long int bytesInQueueM;
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCP_NSC_VIRTUALDATAQUEUES_H

