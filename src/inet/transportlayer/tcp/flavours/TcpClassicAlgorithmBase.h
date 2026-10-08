//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPTAHOERENOFAMILY_H
#define __INET_TCPTAHOERENOFAMILY_H

#include "inet/transportlayer/tcp/flavours/TcpAlgorithmBase.h"
#include "inet/transportlayer/tcp/flavours/TcpClassicAlgorithmBaseState_m.h"

namespace inet {
namespace tcp {

/**
 * The base of the classic loss-based algorithms (TcpReno, TcpNewReno). A flavour
 * is a choice of a congestion control (ITcpCongestionControl) and a recovery
 * (ITcpRecovery); this class drives both from the ACK path and the
 * retransmission timer.
 * (TcpVegas should inherit from TcpAlgorithmBase instead of this one.)
 */
class INET_API TcpClassicAlgorithmBase : public TcpAlgorithmBase
{
  protected:
    TcpClassicAlgorithmBaseStateVariables *& state; // alias to TcpAlgorithm's 'state'

    ITcpCongestionControl *congestionControl = nullptr;
    ITcpRecovery *recovery = nullptr;

  protected:
    virtual ITcpCongestionControl *createCongestionControl() { return nullptr; } // a flavour with its own window growth (TcpCubic) has none
    virtual ITcpRecovery *createRecovery() = 0;

    /**
     * Whether the ACK of new data ends a fast recovery, so the recovery strategy
     * rather than the congestion control handles it. TcpReno answers from the
     * duplicate-ACK counter, which a timeout does not reset.
     */
    virtual bool isInFastRecovery() const { return state->lossRecovery; }

    /**
     * The ECN-Echo reaction on an ACK of new data (RFC 3168): halve cwnd once per
     * round trip. Returns true if it took the place of the window growth.
     * numBytesAcked is for a flavour that weighs its reaction by the acknowledged
     * bytes (DCTCP); RFC 3168 does not use it.
     */
    virtual bool processEce(uint32_t numBytesAcked);

    /**
     * Called after the window update of an ACK of new data, before the sending:
     * TcpReno continues its SACK-based loss recovery here, TcpNewReno moves the
     * recover point.
     */
    virtual void ackProcessed(bool inFastRecovery) {}

    /** The ssthresh that an expired retransmission timer sets: RFC 5681 equation (4), with the outstanding data as FlightSize. */
    virtual uint32_t calculateSsthreshForRto() { return std::max((state->snd_max - state->snd_una) / 2, 2 * state->snd_mss); }

    /** The loss window that an expired retransmission timer restarts slow start from. */
    virtual uint32_t calculateCwndForRto() { return state->snd_mss; }

    /** Redefine what should happen on retransmission */
    virtual void processRexmitTimer(TcpEventCode& event) override;

    /** Called by the base class for each duplicate ACK; the recovery strategy reacts */
    virtual void receivedDuplicateAck() override;

    /** The recovery strategy decides what a duplicate ACK is */
    virtual bool isDuplicateAck(const TcpHeader *tcpHeader, uint32_t payloadLength) override;

  public:
    /** Ctor */
    TcpClassicAlgorithmBase();

    virtual ~TcpClassicAlgorithmBase();

    virtual void initialize() override;

    virtual void established(bool active) override;

    virtual ITcpRecovery *getRecovery() override { return recovery; }

    /** Redefine what should happen when data got acked, to add congestion window management */
    virtual void receivedAckForUnackedData(uint32_t firstSeqAcked) override;

    virtual void receivedAckForAlreadyAckedData(const TcpHeader *tcpHeader, uint32_t payloadLength) override;

    /** Forwarded to the recovery strategy (pre-discard scoreboard inspection). */
    virtual void segmentsAcked(uint32_t fromSeq, uint32_t toSeq) override;

    /**
     * The bytes in flight as Linux counts them: the outstanding data, minus the
     * SACKed and the lost bytes, plus the retransmitted bytes. Without SACK, each
     * duplicate ACK counts as the SACK of one segment.
     */
    virtual uint32_t getBytesInFlight() const override;
};


// Deprecated: TcpTahoeRenoFamily was renamed to TcpClassicAlgorithmBase in INET 4.6, because the old name said
// what the class inherits rather than what it is -- it is the base of the classic loss-based algorithms.
// The alias keeps code outside this repository compiling for one release; it goes
// away in the release after that.
using TcpTahoeRenoFamily = TcpClassicAlgorithmBase;

} // namespace tcp
} // namespace inet

#endif

