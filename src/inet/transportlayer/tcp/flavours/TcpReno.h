//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPRENO_H
#define __INET_TCPRENO_H

#include "inet/transportlayer/tcp/flavours/TcpTahoeRenoFamily.h"

namespace inet {
namespace tcp {

/**
 * State variables for TcpReno.
 */
typedef TcpTahoeRenoFamilyStateVariables TcpRenoStateVariables;

/**
 * Implements TCP Reno.
 */
class INET_API TcpReno : public TcpTahoeRenoFamily
{
  protected:
    TcpRenoStateVariables *& state; // alias to TCLAlgorithm's 'state'

    /** Create and return a TcpRenoStateVariables object. */
    virtual TcpStateVariables *createStateVariables() override
    {
        return new TcpRenoStateVariables();
    }

    /** Utility function to recalculate ssthresh */
    virtual void recalculateSlowStartThreshold();

    /** Redefine what should happen on retransmission */
    virtual void processRexmitTimer(TcpEventCode& event) override;

    /**
     * Fast retransmit + fast recovery entry (the former dupacks==DupThresh
     * branch of receivedDuplicateAck()), shared by the dupack path and the
     * RACK reordering-timer path (rackReoTimeout()).
     */
    virtual void enterFastRecovery();

    /** @name Proportional Rate Reduction (RFC 6937) */
    //@{
    /** RFC 6937 init: snapshot cwnd, reset PRR counters, set ssthresh. Linux tcp_init_cwnd_reduction(). */
    virtual void prrInitCwndReduction();
    /** RFC 6937 per-ACK cwnd sizing: snd_cwnd = pipe + sndcnt. Linux tcp_cwnd_reduction(). */
    virtual void prrCwndReduction(int newlyAckedSacked, int newlyLost, bool sndUnaAdvanced);
    /** RFC 6937 exit: snd_cwnd = ssthresh. Linux tcp_end_cwnd_reduction(). */
    virtual void prrEndCwndReduction();
    /** Newly acked+sacked bytes carried by the ACK currently being processed. */
    uint32_t prrNewlyDelivered() const;
    //@}

    /** @name Loss undo (RFC 2883 D-SACK, RFC 3522 Eifel) */
    //@{
    /** Capture the undo context (marker, priorCwnd/ssthresh) at recovery entry. */
    virtual void undoInit();
    /** True if the cwnd reduction of the current episode may be undone. */
    virtual bool mayUndo() const;
    /** Restore cwnd/ssthresh reduced by a now-known-spurious recovery. Linux tcp_undo_cwnd_reduction(). */
    virtual void undoCwndReduction();
    //@}

  public:
    /** Ctor */
    TcpReno();

    /** TcpReno (and its subclasses TcpCubic, DcTcp) implement RFC 3517 SACK recovery. */
    virtual bool supportsSackRecovery() const override { return true; }

    /** Detect reordering (dynamic DupThresh) when a never-retransmitted segment is acked below the SACK fack. */
    virtual void segmentsAcked(uint32_t fromSeq, uint32_t toSeq) override;

    /** Redefine what should happen when data got acked, to add congestion window management */
    virtual void receivedDataAck(uint32_t firstSeqAcked) override;

    /** Redefine what should happen when dupAck was received, to add congestion window management */
    virtual void receivedDuplicateAck() override;

    /** RACK reordering timer matured with lost-marked data: enter/continue recovery. */
    virtual void rackReoTimeout() override;
};

} // namespace tcp
} // namespace inet

#endif

