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

#ifndef __INET_TCP_NSC_QUEUES_H
#define __INET_TCP_NSC_QUEUES_H

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/tcp_common/TCPSegment.h"

namespace inet {

namespace tcp {

// forward declarations:
class TCP_NSC_Connection;

/**
 * Abstract base class for TCP_NSC send queues. In fact a single object
 * represents both the send queue and the retransmission queue
 * (no need to separate them). The TCPConnection object knows
 * which data in the queue have already been transmitted ("retransmission
 * queue") and which not ("send queue"). This class is not interested
 * in where's the boundary.
 *
 * There is another particularity about this class: as a retransmission queue,
 * it stores bytes and not segments. TCP is a bytestream oriented protocol
 * (sequence numbers refer to bytes and not to TPDUs as e.g. in ISO TP4),
 * and the protocol doesn't rely on retransmitted segments having the
 * same segment boundaries as the original segments. Some implementations
 * store segments on the retransmission queue, and others store only the data
 * bytes; RFCs explicitly allow both. (See e.g. RFC1122 p90, section 4.2.2.15,
 * "IMPLEMENTATION" note).
 *
 * To simulate a TCP that retains segment boundaries in retransmissions,
 * the appropriate TCPAlgorithm class should remember where the segment
 * boundaries were at the original transmission, and it should form identical
 * segments when retransmitting. The createSegmentWithBytes() send queue
 * method makes this possible.
 *
 * This class is polymorphic because depending on where and how you
 * use the TCP model you might have different ideas about "sending data"
 * on a simulated connection.
 *
 * You might want to:
 *
 * - transmit a real bytes, especially if the application which uses TCP
 *   is a ported version of a real socket application.
 *
 * - simulate a "dummy" connection, that is, simulated TCP segments
 *   contain do not contain any real data, only the number of bytes they
 *   represent.  You'll want to do this when the app is there solely
 *   as a traffic generator (e.g. simulated file transfer or telnet session),
 *   but actual data is unimportant.
 *
 * - transmit a sequence of cMessage objects, and you want exactly the
 *   same cMessage sequence to be reproduced on the receiver side.
 *   Here every cMessage maps to a sequence number range in the TCP
 *   stream, and the object is passed up to the application on the
 *   receiving side when its last byte has arrived on the simulated
 *   connection.
 *
 * Different TCP_NSCSendQueue subclasses can be written to accomodate
 * different needs.
 *
 * This class goes hand-in-hand with TCP_NSCReceiveQueue.
 *
 * @see TCP_NSCReceiveQueue
 */

class INET_API TCP_NSC_SendQueue : public cObject
{
  public:
    /**
     * Ctor.
     */
    TCP_NSC_SendQueue() : connM(nullptr) {}

    /**
     * Virtual dtor.
     */
    virtual ~TCP_NSC_SendQueue() {}

    /**
     * set connection queue, and initialise queue variables.
     */
    virtual void setConnection(TCP_NSC_Connection *connP) { connM = connP; }

    /**
     * Called on SEND app command, it inserts in the queue the data the user
     * wants to send. Implementations of this abstract class will decide
     * what this means: copying actual bytes, just increasing the
     * "last byte queued" variable, or storing cMessage object(s).
     * The msg object should not be referenced after this point (sendQueue may
     * delete it.)
     */
    virtual void enqueueAppData(cPacket *msgP) = 0;

    /**
     * Copy data to the buffer for send to NSC.
     * returns lengh of copied data.
     * create msg for socket->send_data()
     *
     * called before called socket->send_data()
     */
    virtual int getBytesForTcpLayer(void *bufferP, int bufferLengthP) const = 0;

    /**
     * The function should remove msgLengthP bytes from NSCqueue
     *
     * But the NSC sometimes reread from this datapart (when data destroyed in IP Layer)
     * inside of createSegmentWithBytes() function.
     *
     * called with return value of socket->send_data() if larger than 0
     */
    virtual void dequeueTcpLayerMsg(int msgLengthP) = 0;

    /**
     * Utility function: returns how many bytes are available in the queue.
     */
    virtual unsigned long getBytesAvailable() const = 0;

    /**
     * Called when the TCP wants to send or retransmit data, it constructs
     * a TCP segment which contains the data from the requested sequence
     * number range. The actually returned segment may contain less then
     * maxNumBytes bytes if the subclass wants to reproduce the original
     * segment boundaries when retransmitting.
     *
     * called from inside of send_callback()
     * called before called the send() to IP layer
     */
    virtual TCPSegment *createSegmentWithBytes(const void *tcpDataP, int tcpLengthP) = 0;

    /**
     * Tells the queue that bytes up to (but NOT including) seqNum have been
     * transmitted and ACKed, so they can be removed from the queue.
     */
    virtual void discardUpTo(uint32 seqNumP) = 0;

  protected:
    TCP_NSC_Connection *connM;
};

class INET_API TCP_NSC_ReceiveQueue : public cObject
{
  public:
    /**
     * Ctor.
     */
    TCP_NSC_ReceiveQueue() : connM(nullptr) {};

    /**
     * Virtual dtor.
     */
    virtual ~TCP_NSC_ReceiveQueue() {}

    /**
     * Add a connection queue.
     */
    virtual void setConnection(TCP_NSC_Connection *connP) { connM = connP; }

    /**
     * Called when a TCP segment arrives. The segment
     * object should *not* be deleted.
     *
     * For remove and store payload messages if need.
     *
     * called before nsc_stack->if_receive_packet() called
     */
    virtual void notifyAboutIncomingSegmentProcessing(TCPSegment *tcpsegP) = 0;

    /**
     * The method called when data received from NSC
     * The method should set status of the data in queue to received
     * called after socket->read_data() successfull
     */
    virtual void enqueueNscData(void *dataP, int dataLengthP) = 0;

    /**
     * Should create a packet to be passed up to the app, up to (but NOT
     * including) the given sequence no (usually rcv_nxt).
     * It should return nullptr if there's no more data to be passed up --
     * this method is called several times until it returns nullptr.
     *
     * called after socket->read_data() successfull
     */
    virtual cPacket *extractBytesUpTo() = 0;

    /**
     * Returns the number of bytes (out-of-order-segments) currently buffered in queue.
     */
    virtual uint32 getAmountOfBufferedBytes() const = 0;

    /**
     * Returns the number of blocks currently buffered in queue.
     */
    virtual uint32 getQueueLength() const = 0;

    /**
     * Shows current queue status.
     */
    virtual void getQueueStatus() const = 0;

    /**
     * notify the queue about output messages
     *
     * called when connM send out a packet.
     * for read AckNo, if have
     */
    virtual void notifyAboutSending(const TCPSegment *tcpsegP) = 0;

  protected:
    TCP_NSC_Connection *connM;
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCP_NSC_QUEUES_H

