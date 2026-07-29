//
// Copyright (C) 2009-2010 Thomas Reschka
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TCPSACKREXMITQUEUE_H
#define __INET_TCPSACKREXMITQUEUE_H

#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {
namespace tcp {

/**
 * Retransmission data for SACK.
 */
class INET_API TcpSackRexmitQueue
{
  public:
    TcpConnection *conn; // the connection that owns this queue

    struct Region {
        uint32_t beginSeqNum;
        uint32_t endSeqNum;
        bool lost; // indicates whether region has been lost
        bool sacked; // indicates whether region has already been sacked by data receiver
        bool rexmitted; // indicates whether region has already been retransmitted by data sender
        simtime_t firstSentTime = 0; // time this region was first transmitted (RACK/Vegas: original send time)
        simtime_t lastSentTime = 0; // time this region was most recently (re)transmitted (RACK: xmit time)
        uint16_t transmitCount = 0; // number of times this region has been transmitted (1 = never retransmitted)
    };

    typedef std::list<Region> RexmitQueue;
    RexmitQueue rexmitQueue; // rexmitQueue is ordered by seqnum, and doesn't have overlapped Regions
    std::set<uint32_t> xmitSegmentStarts; // begin seqnums of ORIGINAL transmissions (skb boundaries): lets RACK tell a whole small segment (advances the reference, Linux tags the skb) from a sub-MSS fragment split off a bigger segment by a byte-range SACK (never tagged, tcp_match_skb_to_sack fragments only at MSS boundaries)

    bool isTransmissionStart(uint32_t seqNum) const { return xmitSegmentStarts.find(seqNum) != xmitSegmentStarts.end(); }

    uint32_t begin; // 1st sequence number stored
    uint32_t end; // last sequence number stored + 1

  protected:
    // getLost()/getSacked()/getRetrans() are read several times per ACK (setPipe alone
    // runs 2-3 times, getBytesInFlight sums all three), so the totals are walked once
    // and cached until something touches the queue. Linux keeps the equivalent
    // lost_out/sacked_out/retrans_out permanently up to date at every mutation point;
    // invalidating is the same idea with one place to get right instead of twenty.
    mutable bool countersValid = false;
    mutable uint32_t lostBytes = 0;
    mutable uint32_t sackedBytes = 0;
    mutable uint32_t retransBytes = 0;

    /** Walks the queue once to refresh the cached flag totals. */
    virtual void updateCounters() const;

    /** Every insertion, removal or flag change in the queue must call this. */
    void invalidateCounters() { countersValid = false; }

  public:
    /**
     * Ctor
     */
    TcpSackRexmitQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TcpSackRexmitQueue();

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
    virtual void init(uint32_t seqNum);

    /**
     * Returns a string for debug purposes.
     */
    virtual std::string str() const;

    /**
     * Prints the current rexmitQueue status for debug purposes.
     */
    virtual std::string detailedInfo() const;

    /**
     * Returns the sequence number of the first byte stored in the buffer.
     */
    virtual uint32_t getBufferStartSeq() const { return begin; }

    /**
     * Returns the sequence number of the last byte stored in the buffer plus one.
     * (The first byte of the next send operation would get this sequence number.)
     */
    virtual uint32_t getBufferEndSeq() const { return end; }

    /**
     * Tells the queue that bytes up to (but NOT including) seqNum have been
     * transmitted and ACKed, so they can be removed from the queue.
     */
    virtual void discardUpTo(uint32_t seqNum);

    /**
     * Inserts sent data to the rexmit queue.
     */
    virtual void enqueueSentData(uint32_t fromSeqNum, uint32_t toSeqNum);

    /**
     * Emulate sacks for sackless connections. Called on a new dupack, it marks
     * one more segment as sacked.
     */
    virtual void addInferredSack();

    /**
     * Called when data sender received selective acknowledgments.
     * Tells the queue which bytes have been transmitted and SACKed,
     * so they can be skipped if retransmitting segments as long as
     * REXMIT timer did not expired.
     */
    /**
     * Marks [fromSeqNum, toSeqNum) as SACKed. Returns the lowest sequence number
     * this call NEWLY marked (skipping ever-retransmitted regions, whose SACKs are
     * ambiguous), or 0 if nothing was newly marked -- used for reordering detection.
     */
    virtual uint32_t setSackedBit(uint32_t fromSeqNum, uint32_t toSeqNum);

    /**
     * Returns SackedBit value of seqNum.
     */
    virtual bool getSackedBit(uint32_t seqNum) const;

    /**
     * Returns the number of blocks currently buffered in queue.
     */
    virtual uint32_t getQueueLength() const { return rexmitQueue.size(); }

    /**
     * Returns the highest sequence number sacked by data receiver.
     */
    virtual uint32_t getHighestSackedSeqNum() const;

    /**
     * Returns the highest sequence number rexmitted by data sender.
     */
    virtual uint32_t getHighestRexmittedSeqNum() const;

    /**
     * Checks rexmit queue for sacked of rexmitted segments and returns a certain offset
     * (contiguous sacked or rexmitted region) to forward snd->nxt.
     * It is called before retransmitting data.
     */
    virtual uint32_t checkRexmitQueueForSackedOrRexmittedSegments(uint32_t fromSeq) const;

    virtual void markHeadLost();

    /**
     * Resets lost bit of all segments in rexmit queue.
     */
    virtual void resetLostBit();

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
     * Returns total amount of sacked bytes. Corresponds to update() function from RFC 6675.
     */
    virtual uint32_t getTotalAmountOfSackedBytes() const;

    /**
     * Returns amount of sacked bytes above seqNum.
     */
    virtual uint32_t getAmountOfSackedBytes(uint32_t seqNum) const;

    /**
     * Returns the number of discontiguous sacked regions (SACKed sequences) above seqNum.
     */
    virtual uint32_t getNumOfDiscontiguousSacks(uint32_t seqNum) const;

    /*
     * Returns nothing but checks length, sacked bit and rexmitted bit of a given
     * SACK block starting at seqNum.
     */
    virtual void checkSackBlock(uint32_t seqNum, uint32_t& length, bool& sacked, bool& rexmitted) const;

    virtual void updateLost();

    /**
     * Returns the total number of lost bytes in the queue.
     */
    virtual uint32_t getLost() const;

    /**
     * Returns the total number of sacked bytes in the queue.
     */
    virtual uint32_t getSacked() const;

    /**
     * Returns the total number of retransmitted bytes in the queue.
     */
    virtual uint32_t getRetrans() const;

    /**
     * Returns the region containing seqNum. seqNum must be within [begin, end).
     * Used by RACK to read a segment's transmit time and count.
     */
    virtual const Region& getRegion(uint32_t seqNum) const;

    /**
     * Marks the byte range [fromSeqNum, toSeqNum) as lost (RACK/RFC 3517),
     * splitting regions at the boundaries as needed.
     */
    virtual void markLost(uint32_t fromSeqNum, uint32_t toSeqNum);

    /** RACK re-marked a lost RETRANSMISSION: clear the rexmitted flag on unsacked regions in the range (Linux tcp_mark_skb_lost clearing TCPCB_SACKED_RETRANS) so the recovery picker sends them again; the lost mark stays. */
    virtual void clearRexmitted(uint32_t fromSeqNum, uint32_t toSeqNum);

  protected:
    /*
     * Returns if TcpSackRexmitQueue is valid or not.
     */
    bool checkQueue() const;
};

} // namespace tcp
} // namespace inet

#endif

