//
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_TCPSENDQUEUE_H
#define __INET_TCPSENDQUEUE_H

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/tcp/TCPConnection.h"
#include "inet/transportlayer/tcp_common/TCPSegment.h"

namespace inet {

namespace tcp {

/**
 * Abstract base class for TCP send queues. In fact a single object
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
 * bytes; RFCs explicitly allow both. (See e.g. RFC 1122 p90, section 4.2.2.15,
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
 *   represent. You'll want to do this when the app is there solely
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
 * Different TCPSendQueue subclasses can be written to accomodate
 * different needs.
 *
 * This class goes hand-in-hand with TCPReceiveQueue.
 *
 * @see TCPReceiveQueue
 */
class INET_API TCPSendQueue : public cObject
{
  protected:
    TCPConnection *conn;    // the connection that owns this queue

  public:
    /**
     * Ctor.
     */
    TCPSendQueue() { conn = nullptr; }

    /**
     * Virtual dtor.
     */
    virtual ~TCPSendQueue() {}

    /**
     * Set the connection that owns this queue.
     */
    virtual void setConnection(TCPConnection *_conn) { conn = _conn; }

    /**
     * Initialize the object. The startSeq parameter tells what sequence number the first
     * byte of app data should get. This is usually ISS + 1 because SYN consumes
     * one byte in the sequence number space.
     *
     * init() may be called more than once; every call flushes the existing contents
     * of the queue.
     */
    virtual void init(uint32 startSeq) = 0;

    /**
     * Called on SEND app command, it inserts in the queue the data the user
     * wants to send. Implementations of this abstract class will decide
     * what this means: copying actual bytes, just increasing the
     * "last byte queued" variable, or storing cMessage object(s).
     * The msg object should not be referenced after this point (sendQueue may
     * delete it.)
     */
    virtual void enqueueAppData(cPacket *msg) = 0;

    /**
     * Returns the sequence number of the first byte stored in the buffer.
     */
    virtual uint32 getBufferStartSeq() = 0;

    /**
     * Returns the sequence number of the last byte stored in the buffer plus one.
     * (The first byte of the next send operation would get this sequence number.)
     */
    virtual uint32 getBufferEndSeq() = 0;

    /**
     * Utility function: returns how many bytes are available in the queue, from
     * (and including) the given sequence number.
     */
    inline ulong getBytesAvailable(uint32 fromSeq)
    {
        uint32 bufEndSeq = getBufferEndSeq();
        return seqLess(fromSeq, bufEndSeq) ? bufEndSeq - fromSeq : 0;
    }

    /**
     * Called when the TCP wants to send or retransmit data, it constructs
     * a TCP segment which contains the data from the requested sequence
     * number range. The actually returned segment may contain less than
     * maxNumBytes bytes if the subclass wants to reproduce the original
     * segment boundaries when retransmitting.
     */
    virtual TCPSegment *createSegmentWithBytes(uint32 fromSeq, ulong maxNumBytes) = 0;

    /**
     * Tells the queue that bytes up to (but NOT including) seqNum have been
     * transmitted and ACKed, so they can be removed from the queue.
     */
    virtual void discardUpTo(uint32 seqNum) = 0;
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCPSENDQUEUE_H

