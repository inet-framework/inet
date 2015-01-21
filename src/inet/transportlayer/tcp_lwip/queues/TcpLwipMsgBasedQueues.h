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

#ifndef __INET_TCPLWIPMSGBASEDQUEUES_H
#define __INET_TCPLWIPMSGBASEDQUEUES_H

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/tcp_lwip/queues/TcpLwipQueues.h"
#include "inet/transportlayer/tcp_common/TCPSegment.h"

namespace inet {

namespace tcp {

/**
 * Send queue that manages "objects".
 */
class INET_API TcpLwipMsgBasedSendQueue : public TcpLwipSendQueue
{
  protected:
    struct Payload
    {
        unsigned int endSequenceNo;
        cPacket *msg;
    };

    typedef std::list<Payload> PayloadQueue;

    PayloadQueue payloadQueueM;

    uint32 beginM;    // 1st sequence number stored
    uint32 endM;    // last sequence number stored +1
    bool isValidSeqNoM;
    unsigned long int unsentTcpLayerBytesM;

  public:
    /**
     * Ctor.
     */
    TcpLwipMsgBasedSendQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TcpLwipMsgBasedSendQueue();

    /**
     * set connection queue.
     */
    virtual void setConnection(TcpLwipConnection *connP) override;

    /**
     * Called on SEND app command, it inserts in the queue the data the user
     * wants to send. Implementations of this abstract class will decide
     * what this means: copying actual bytes, just increasing the
     * "last byte queued" variable, or storing cMessage object(s).
     * The msg object should not be referenced after this point (sendQueue may
     * delete it.)
     */
    virtual void enqueueAppData(cPacket *msgP) override;

    /**
     * Copy data to the buffer for send to LWIP.
     * returns lengh of copied data.
     * create msg for socket->send_data()
     *
     * called before called socket->send_data()
     */
    virtual unsigned int getBytesForTcpLayer(void *bufferP, unsigned int bufferLengthP) const override;

    /**
     * Remove msgLengthP bytes from TCP Layer queue
     *
     * called with return value of socket->send_data() if larger than 0
     */
    virtual void dequeueTcpLayerMsg(unsigned int msgLengthP) override;

    /**
     * Utility function: returns how many bytes are available in the queue.
     */
    unsigned long getBytesAvailable() const override;

    /**
     * Called when the TCP wants to send or retransmit data, it constructs
     * a TCP segment which contains the data from the requested sequence
     * number range. The actually returned segment may contain less then
     * maxNumBytes bytes if the subclass wants to reproduce the original
     * segment boundaries when retransmitting.
     *
     * called from inside of send_callback()
     * called before called the send() to IP layer
     * @param tcpDataP: the tcp segment (with tcp header) created by LWIP
     * @param tcpLenthP: the length of tcp segment.
     */
    virtual TCPSegment *createSegmentWithBytes(const void *tcpDataP, unsigned int tcpLengthP) override;

    /**
     * Tells the queue that bytes up to (but NOT including) seqNum have been
     * transmitted and ACKed, so they can be removed from the queue.
     */
    virtual void discardAckedBytes();
};

/**
 * Receive queue that manages "objects".
 */
class INET_API TcpLwipMsgBasedReceiveQueue : public TcpLwipReceiveQueue
{
  protected:
    struct PayloadItem
    {
        uint32 seqNo;
        cPacket *packet;
        PayloadItem(uint32 _seqNo, cPacket *_packet) : seqNo(_seqNo), packet(_packet) {}
    };
    typedef std::list<PayloadItem> PayloadList;
    PayloadList payloadListM;
    bool isValidSeqNoM;
    uint32 lastExtractedSeqNoM;

  public:
    /**
     * Ctor.
     */
    TcpLwipMsgBasedReceiveQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TcpLwipMsgBasedReceiveQueue();

    /**
     * Set the connection.
     */
    virtual void setConnection(TcpLwipConnection *connP) override;

    /**
     * called back from lwip::tcp_input()
     */
    virtual void notifyAboutIncomingSegmentProcessing(TCPSegment *tcpsegP, uint32 seqNo,
            const void *bufferP, size_t bufferLengthP) override;

    /**
     * The method called when data received from LWIP
     * The method should set status of the data in queue to received
     * called after socket->read_data() successful
     */
    virtual void enqueueTcpLayerData(void *dataP, unsigned int dataLengthP) override;

    /**
     * Should create a packet to be passed up to the app, up to (but NOT
     * including) the given sequence no (usually rcv_nxt).
     * It should return nullptr if there's no more data to be passed up --
     * this method is called several times until it returns nullptr.
     *
     * called after socket->read_data() successful
     */
    virtual cPacket *extractBytesUpTo() override;

    /**
     * Returns the number of bytes (out-of-order-segments) currently buffered in queue.
     */
    virtual uint32 getAmountOfBufferedBytes() const override;

    /**
     * Returns the number of blocks currently buffered in queue.
     */
    virtual uint32 getQueueLength() const override;

    /**
     * Shows current queue status.
     */
    virtual void getQueueStatus() const override;

    /**
     * notify the queue about output messages
     *
     * called when connM send out a packet.
     * for read AckNo, if have
     */
    virtual void notifyAboutSending(const TCPSegment *tcpsegP) override;

  protected:
    long int bytesInQueueM;
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCPLWIPMSGBASEDQUEUES_H

