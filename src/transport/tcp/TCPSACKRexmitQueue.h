//
// Copyright (C) 2009-2010 Thomas Reschka
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

#ifndef __INET_TCPSACKREXMITQUEUE_H
#define __INET_TCPSACKREXMITQUEUE_H

#include <omnetpp.h>
#include "TCPConnection.h"
#include "TCPSegment.h"


/**
 * Retransmission data for SACK.
 */
class INET_API TCPSACKRexmitQueue
{
  public:
    TCPConnection *conn; // the connection that owns this queue

    struct Region
    {
        uint32 beginSeqNum;
        uint32 endSeqNum;
        bool sacked;      // indicates whether region has already been sacked by data receiver
        bool rexmitted;   // indicates whether region has already been retransmitted by data sender
    };
    typedef std::list<Region> RexmitQueue;
    RexmitQueue rexmitQueue;

    uint32 begin;  // 1st sequence number stored
    uint32 end;    // last sequence number stored +1

  public:
    /**
     * Ctor
     */
    TCPSACKRexmitQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TCPSACKRexmitQueue();

    /**
     * Set the connection that owns this queue.
     */
    virtual void setConnection(TCPConnection *_conn)  {conn = _conn;}

    /**
     * Initialize the object. The startSeq parameter tells what sequence number the first
     * byte of app data should get. This is usually ISS+1 because SYN consumes
     * one byte in the sequence number space.
     *
     * init() may be called more than once; every call flushes the existing contents
     * of the queue.
     */
    virtual void init(uint32 seqNum);

    /**
     * Returns a string for debug purposes.
     */
    virtual std::string str() const;

    /**
     * Prints the current rexmitQueue status for debug purposes.
     */
    virtual void info();

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
     * Tells the queue that bytes up to (but NOT including) seqNum have been
     * transmitted and ACKed, so they can be removed from the queue.
     */
    virtual void discardUpTo(uint32 seqNum);

    /**
     * Inserts sent data to the rexmit queue.
     */
    virtual void enqueueSentData(uint32 fromSeqNum, uint32 toSeqNum);

    /**
     * Called when data sender received selective acknowledgments.
     * Tells the queue which bytes have been transmitted and SACKed,
     * so they can be skipped if retransmitting segments as long as
     * REXMIT timer did not expired.
     */
    virtual void setSackedBit(uint32 fromSeqNum, uint32 toSeqNum);

    /**
     * Returns SackedBit value of seqNum.
     */
    virtual bool getSackedBit(uint32 seqNum);

    /**
     * Returns the number of blocks currently buffered in queue.
     */
    virtual uint32 getQueueLength();

    /**
     * Returns the highest sequence number sacked by data receiver.
     */
    virtual uint32 getHighestSackedSeqNum();

    /**
     * Returns the highest sequence number rexmitted by data sender.
     */
    virtual uint32 getHighestRexmittedSeqNum();

    /**
     * Checks rexmit queue for sacked of rexmitted segments and returns a certain offset
     * (contiguous sacked or rexmitted region) to forward snd->nxt.
     * It is called before retransmitting data.
     */
    virtual uint32 checkRexmitQueueForSackedOrRexmittedSegments(uint32 fromSeq);

    /**
     * Called when REXMIT timer expired.
     * Resets sacked bit of all segments in rexmit queue.
     */
    virtual void resetSackedBit();

    /**
     * Called when REXMIT timer expired.
     * Resets rexmitted bit of all segments in rexmit queue.
     */
    virtual void resetRexmittedBit();

    /**
     * Returns total amount of sacked bytes. Corresponds to update() function from RFC 3517.
     */
    virtual uint32 getTotalAmountOfSackedBytes();

    /**
     * Returns amount of sacked bytes above seqNum.
     */
    virtual uint32 getAmountOfSackedBytes(uint32 seqNum);

    /**
     * Returns the number of discontiguous sacked regions (SACKed sequences) above seqNum.
     */
    virtual uint32 getNumOfDiscontiguousSacks(uint32 seqNum);
};

#endif
