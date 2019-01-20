//
// Copyright (C) 2004 - 2016 OpenSim Ltd.
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
#include "inet/common/packet/ChunkQueue.h"
#include "inet/common/packet/Packet.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {
namespace tcp {

/**
 * Send queue that manages Chunks.
 *
 * @see TCPRcvQueue
 */
class INET_API TcpSendQueue : public cObject
{
  protected:
    TcpConnection *conn = nullptr;    // the connection that owns this queue
    uint32 begin = 0;    // 1st sequence number stored
    uint32 end = 0;    // last sequence number stored +1
    ChunkQueue dataBuffer;      // dataBuffer

  public:
    /**
     * Ctor
     */
    TcpSendQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TcpSendQueue();

    virtual ChunkQueue& getDataBuffer() { return dataBuffer; }

    /**
     * Set the connection that owns this queue.
     */
    virtual void setConnection(TcpConnection *_conn) { conn = _conn; }

    /**
     * Initialize the object. The startSeq parameter tells what sequence number the first
     * byte of app data should get. This is usually ISS + 1 because SYN consumes
     * one byte in the sequence number space.
     *
     * init() may be called more than once; every call flushes the existing contents
     * of the queue.
     */
    virtual void init(uint32 startSeq);

    /**
     * Returns a string with the region stored.
     */
    virtual std::string str() const override;

    /**
     * Called on SEND app command, it inserts in the queue the data the user
     * wants to send. Implementations of this abstract class will decide
     * what this means: copying actual bytes, just increasing the
     * "last byte queued" variable, or storing cMessage object(s).
     * The msg object should not be referenced after this point (sendQueue may
     * delete it.)
     */
    virtual void enqueueAppData(Packet *msg);

    /**
     * Returns the sequence number of the first byte stored in the buffer.
     */
    virtual uint32 getBufferStartSeq();

    /**
     * Returns the sequence number of the last byte stored in the buffer plus one.
     * (The first byte of the next send operation would get this sequence number.)
     */
    virtual uint32 getBufferEndSeq();

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
    virtual Packet *createSegmentWithBytes(uint32 fromSeq, ulong numBytes);

    /**
     * Tells the queue that bytes up to (but NOT including) seqNum have been
     * transmitted and ACKed, so they can be removed from the queue.
     */
    virtual void discardUpTo(uint32 seqNum);
};

} // namespace tcp
} // namespace inet

#endif // ifndef __INET_TCPSENDQUEUE_H

