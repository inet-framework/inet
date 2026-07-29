//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TCPCUBIC_H
#define __INET_TCPCUBIC_H

#include "inet/transportlayer/tcp/flavours/TcpAlgorithmBase.h"
#include "inet/transportlayer/tcp/flavours/TcpCubicState_m.h"

namespace inet {
namespace tcp {

/**
 * Implements CUBIC congestion control (RFC 9438), modelled on Linux's
 * tcp_cubic.c, including the HyStart hybrid slow-start exit.
 *
 * CUBIC replaces Reno's linear congestion-avoidance growth with a cubic
 * function of the time elapsed since the last window reduction. The window
 * climbs quickly back towards W_max (the window at which loss was detected),
 * flattens out around it, and then probes for more capacity -- so the growth
 * rate no longer depends on the RTT, which is what makes CUBIC fair across
 * paths of different lengths.
 *
 * Loss recovery is orthogonal to the growth law: this class delegates it to
 * an RFC 6675 (SACK) or RFC 6582 (NewReno) recovery strategy, and contributes
 * only the multiplicative decrease (cwnd * beta, with fast convergence).
 */
class INET_API TcpCubic : public TcpAlgorithmBase
{
  public:
    /** HyStart exit detectors; the hystartDetect parameter is a bitmask of these. */
    enum HystartDetect {
        HYSTART_ACK_TRAIN = 1, ///< ACKs arriving in a train longer than the min RTT
        HYSTART_DELAY = 2, ///< the round's minimum RTT rising above the connection minimum
    };

  protected:
    TcpCubicStateVariables *& state; // alias to TcpAlgorithm's 'state'
    ITcpRecovery *recovery = nullptr;

  protected:
    virtual TcpStateVariables *createStateVariables() override
    {
        return new TcpCubicStateVariables();
    }

    /** Picks the loss-recovery strategy matching the negotiated SACK support. */
    virtual ITcpRecovery *createRecovery();

    /** Forgets the epoch and the W_max memory (Linux bictcp_reset). */
    virtual void cubicReset();

    /** Starts a fresh HyStart round (Linux bictcp_hystart_reset). */
    virtual void hystartReset();

    /** Grows cwnd by the segments this ACK acknowledged, while cwnd-limited. */
    virtual void slowStart(uint32_t segmentsAcked);

    /** Grows cwnd along the cubic curve, one SMSS per cubic_cnt segments acked. */
    virtual void congestionAvoidance(uint32_t segmentsAcked);

    /**
     * Recomputes cubic_cnt, the number of acked segments that must accumulate
     * before cwnd may grow by one SMSS (Linux bictcp_update), and returns it.
     */
    virtual uint32_t cubicUpdate(uint32_t segmentsAcked);

    /**
     * Times the first newly acknowledged byte against now and feeds that raw
     * per-ACK sample to the min-RTT tracker and to HyStart, the way Linux fills
     * ack_sample::rtt_us for the flavour's pkts_acked hook. Silent when the sample
     * would be ambiguous (a retransmitted segment).
     */
    virtual void processAckRttSample(uint32_t firstSeqAcked);

    /** Feeds an RTT sample to the min-RTT tracker and to HyStart. */
    virtual void processRttSample(const simtime_t& rtt);

    /** Looks for the slow-start exit point (Linux hystart_update). */
    virtual void hystartUpdate(const simtime_t& delay);

    /** Clamps the delay-increase threshold into the configured range. */
    virtual simtime_t hystartDelayThresh(const simtime_t& t) const;

  public:
    TcpCubic();
    virtual ~TcpCubic();

    virtual void initialize() override;
    virtual void established(bool active) override;
    virtual ITcpRecovery *getRecovery() override;

    virtual void processRexmitTimer(TcpEventCode& event) override;
    virtual bool isDuplicateAck(const TcpHeader *tcpHeader, uint32_t payloadLength) override;
    virtual void receivedAckForUnackedData(uint32_t firstSeqAcked) override;
    virtual void receivedDuplicateAck() override;
    virtual uint32_t getBytesInFlight() const override;

    /** TcpCubic selects RFC 6675 SACK recovery when the connection negotiated SACK. */
    virtual bool supportsSackRecovery() const override { return true; }

    /**
     * The CUBIC multiplicative decrease: remembers the window as the new W_max
     * (applying fast convergence), ends the epoch, and returns beta * cwnd.
     * bytesInFlight is reported only; the reduction is taken from cwnd, as in
     * Linux cubictcp_recalc_ssthresh.
     */
    virtual uint32_t calculateSsthresh(uint32_t bytesInFlight);

    /** CUBIC reduces to cwnd*beta on fast-recovery entry, not to FlightSize/2. */
    virtual uint32_t calculateSsthreshForFastRecovery() override { return calculateSsthresh(state->snd_cwnd); }
};

} // namespace tcp
} // namespace inet

#endif // __INET_TCPCUBIC_H
