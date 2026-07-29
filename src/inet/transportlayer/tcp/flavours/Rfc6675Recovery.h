//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_RFC6675RECOVERY_H
#define __INET_RFC6675RECOVERY_H

#include "inet/transportlayer/tcp/ITcpRecovery.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/flavours/TcpClassicAlgorithmBase.h"

namespace inet {
namespace tcp {

/**
 * Implements RFC 6675: A Conservative Loss Recovery Algorithm Based on Selective Acknowledgment (SACK) for TCP.
 */
class INET_API Rfc6675Recovery : public ITcpRecovery
{
  protected:
    TcpClassicAlgorithmBaseStateVariables *state = nullptr;
    TcpConnection *conn = nullptr;

    virtual void stepA();
    virtual void stepB();
    virtual void stepC();

    virtual void step4();

  public:
    Rfc6675Recovery(TcpStateVariables *state, TcpConnection *conn) : state(check_and_cast<TcpClassicAlgorithmBaseStateVariables *>(state)), conn(conn) { }

    virtual bool isDuplicateAck(const TcpHeader *tcpHeader, uint32_t payloadLength) override;

    virtual void receivedAckForUnackedData(uint32_t numBytesAcked) override;

    virtual void receivedDuplicateAck() override;

    virtual bool processSACKOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionSack& option);
    /**
     * For SACK TCP. RFC 6675, page 5: "This routine returns whether the given
     * sequence number is considered to be lost.  The routine returns true when
     * either DupThresh discontiguous SACKed sequences have arrived above
     * 'SeqNum' or (DupThresh - 1) * SMSS bytes with sequence numbers greater
     * than 'SeqNum' have been SACKed.  Otherwise, the routine returns
     * false."
     */
    virtual bool isLost(uint32_t seqNum);

    /**
     * RFC 8985 RACK: advance the RACK reference to the most recently sent
     * delivered segment and mark earlier-sent, still-unacked segments as lost
     * once RACK.rtt + reo_wnd has elapsed. Returns the number of newly lost bytes.
     */
    virtual uint32_t rackDetectAndMarkLost(bool fromReoTimer = false);

    /** @name Proportional Rate Reduction (RFC 6937), Linux tcp_cwnd_reduction() */
    //@{
    /** Newly acked+sacked bytes carried by the ACK currently being processed. */
    virtual uint32_t prrNewlyDelivered() const;
    /** Per-ACK cwnd sizing: snd_cwnd = pipe + sndcnt. */
    virtual void prrCwndReduction(int newlyAckedSacked, int newlyLost, bool sndUnaAdvanced);
    /** Recovery exit: snd_cwnd = ssthresh. */
    virtual void prrEndCwndReduction();
    //@}

    /** @name Loss undo (RFC 2883 D-SACK, RFC 3522 Eifel), Linux tcp_undo_cwnd_reduction() */
    //@{
    /** Capture the undo context (marker, priorCwnd/priorSsthresh) at recovery entry. */
    virtual void undoInit();
    /** True if the cwnd reduction of the current episode may be undone. */
    /** Eifel (RFC 3522): the last ACK's TSecr predates our first retransmission. */
    virtual bool packetDelayed() const;
    virtual bool mayUndo() const;
    /** Restore cwnd/ssthresh reduced by a now-known-spurious recovery. */
    virtual void undoCwndReduction();
    //@}

    /**
     * Linux tcp_check_sack_reordering(): reordering is proven when data at lowSeq
     * was delivered while a higher sequence number (the SACK fack) had already been
     * SACKed. Grows the learned reordering degree, bounded by maxReordering.
     */
    virtual void checkSackReordering(uint32_t lowSeq);

    /** RFC 5682 F-RTO: decide/close a spurious-RTO episode. */
    virtual void processFrtoEpisode();

    virtual void onRexmitTimeout() override;
    virtual void reoTimeout() override;
    virtual void segmentsAcked(uint32_t fromSeq, uint32_t toSeq) override;
    virtual void dataSent(uint32_t fromSeq) override;
    virtual void segmentRetransmitted(uint32_t fromSeq, uint32_t toSeq) override;

    /**
     * For SACK TCP. RFC 6675, page 5: "This routine traverses the sequence
     * space from HighACK to HighData and MUST set the "pipe" variable to an
     * estimate of the number of octets that are currently in transit between
     * the TCP sender and the TCP receiver."
     */
    virtual void setPipe();

    /**
     * For SACK TCP. RFC 6675, page 6: "This routine uses the scoreboard data
     * structure maintained by the Update() function to determine what to transmit
     * based on the SACK information that has arrived from the data receiver
     * (and hence been marked in the scoreboard).  NextSeg () MUST return the
     * sequence number range of the next segment that is to be
     * transmitted..."
     * Returns true if a valid sequence number (for the next segment) is found and
     * returns false if no segment should be send.
     */
    virtual bool nextSeg(uint32_t& seqNum);

    /**
     * Utility: send data during Loss Recovery phase (if SACK is enabled).
     */
    virtual void sendDataDuringLossRecoveryPhase(uint32_t congestionWindow);

    /**
     * Utility: send segment during Loss Recovery phase (if SACK is enabled).
     * Returns the number of bytes sent.
     */
    virtual uint32_t sendSegmentDuringLossRecoveryPhase(uint32_t seqNum);

    /** Utility: adds SACKs to segments header options field */
    virtual TcpHeader addSacks(const Ptr<TcpHeader>& tcpHeader);
};

} // namespace tcp
} // namespace inet

#endif

