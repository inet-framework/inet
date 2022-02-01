//
// Copyright (C) 2004 - 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPSENDQUEUE_H
#define __INET_TCPSENDQUEUE_H

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
    TcpConnection *conn = nullptr; // the connection that owns this queue
    uint32_t begin = 0; // 1st sequence number stored
    uint32_t end = 0; // last sequence number stored +1
    ChunkQueue dataBuffer; // dataBuffer

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
    virtual void init(uint32_t startSeq);

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
    virtual uint32_t getBufferStartSeq() const;

    /**
     * Returns the sequence number of the last byte stored in the buffer plus one.
     * (The first byte of the next send operation would get this sequence number.)
     */
    virtual uint32_t getBufferEndSeq() const;

    /**
     * Utility function: returns how many bytes are available in the queue, from
     * (and including) the given sequence number.
     */
    virtual uint32_t getBytesAvailable(uint32_t fromSeq) const;

    /**
     * Called when the TCP wants to send or retransmit data, it constructs
     * a TCP segment which contains the data from the requested sequence
     * number range. The actually returned segment may contain less than
     * maxNumBytes bytes if the subclass wants to reproduce the original
     * segment boundaries when retransmitting.
     */
    virtual Packet *createSegmentWithBytes(uint32_t fromSeq, uint32_t numBytes);

    /**
     * Tells the queue that bytes up to (but NOT including) seqNum have been
     * transmitted and ACKed, so they can be removed from the queue.
     */
    virtual void discardUpTo(uint32_t seqNum);
};

} // namespace tcp
} // namespace inet

#endif

