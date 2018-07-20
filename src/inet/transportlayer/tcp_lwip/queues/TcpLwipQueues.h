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

#ifndef __INET_TCPLWIPQUEUES_H
#define __INET_TCPLWIPQUEUES_H

#include "inet/common/INETDefs.h"
#include "inet/common/packet/ChunkQueue.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BytesChunk.h"

namespace inet {

namespace tcp {

// forward declarations:
class TcpLwipConnection;
class TcpHeader;

/**
 * TCP_LWIP send queue. In fact a single object
 * represents both the send queue and the retransmission queue
 * (no need to separate them). The TcpConnection object knows
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
 * the appropriate TcpAlgorithm class should remember where the segment
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
 * Different TcpLwipSendQueue subclasses can be written to accomodate
 * different needs.
 *
 * This class goes hand-in-hand with TcpLwipReceiveQueue.
 *
 * @see TcpLwipReceiveQueue
 */

class INET_API TcpLwipSendQueue : public cObject
{
  public:
    /**
     * Ctor.
     */
    TcpLwipSendQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TcpLwipSendQueue();

    /**
     * set connection queue, and initialise queue variables.
     */
    virtual void setConnection(TcpLwipConnection *connP);

    /**
     * Called on SEND app command, it inserts in the queue the data the user
     * wants to send. Implementations of this abstract class will decide
     * what this means: copying actual bytes, just increasing the
     * "last byte queued" variable, or storing cMessage object(s).
     * The msg object should not be referenced after this point (sendQueue may
     * delete it.)
     */
    virtual void enqueueAppData(Packet *msgP);

    /**
     * Copy data to the buffer for send to LWIP.
     * returns lengh of copied data.
     * create msg for socket->send_data()
     *
     * called before called socket->send_data()
     */
    virtual unsigned int getBytesForTcpLayer(void *bufferP, unsigned int bufferLengthP) const;

    /**
     * This function should remove msgLengthP bytes from TCP layer queue
     */
    virtual void dequeueTcpLayerMsg(unsigned int msgLengthP);

    /**
     * Utility function: returns how many bytes are available in the queue.
     */
    virtual unsigned long getBytesAvailable() const;

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
    virtual Packet *createSegmentWithBytes(const void *tcpDataP, unsigned int tcpLengthP);

    virtual void discardAckedBytes(unsigned long bytesP);

  protected:
    TcpLwipConnection *connM = nullptr;
    ChunkQueue dataBuffer;      // dataBuffer
};

class INET_API TcpLwipReceiveQueue : public cObject
{
  public:
    /**
     * Ctor.
     */
    TcpLwipReceiveQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TcpLwipReceiveQueue();

    /**
     * Add a connection queue.
     */
    virtual void setConnection(TcpLwipConnection *connP);

    /**
     * Called when a TCP segment arrives, it should extract the payload
     * from the segment and store it in the receive queue. The segment
     * object should *not* be deleted.
     * //FIXME revise this comment
     */
    virtual void notifyAboutIncomingSegmentProcessing(Packet *packet, uint32 seqNo,
            const void *bufferP, size_t bufferLengthP);

    /**
     * The method called when data received from LWIP
     * The method should set status of the data in queue to received
     * called after socket->read_data() successfull
     */
    virtual void enqueueTcpLayerData(void *dataP, unsigned int dataLengthP);

    virtual unsigned long getExtractableBytesUpTo() const;

    /**
     * Should create a packet to be passed up to the app, up to (but NOT
     * including) the given sequence no (usually rcv_nxt).
     * It should return nullptr if there's no more data to be passed up --
     * this method is called several times until it returns nullptr.
     *
     * called after socket->read_data() successfull
     */
    virtual Packet *extractBytesUpTo();

    /**
     * Returns the number of bytes (out-of-order-segments) currently buffered in queue.
     */
    virtual uint32 getAmountOfBufferedBytes() const;

    /**
     * Returns the number of blocks currently buffered in queue.
     */
    virtual uint32 getQueueLength() const;

    /**
     * Shows current queue status.
     */
    virtual void getQueueStatus() const;

    /**
     * notify the queue about output messages
     *
     * called when connM send out a packet.
     * for read AckNo, if have
     */
    virtual void notifyAboutSending(const TcpHeader *tcpsegP);

  protected:
    TcpLwipConnection *connM = nullptr;
    ChunkQueue dataBuffer;      // fifo dataBuffer
    // ReorderBuffer reorderBuffer;     // ReorderBuffer not needed, because the original lwIP code manages the unordered buffer
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCPLWIPQUEUES_H

