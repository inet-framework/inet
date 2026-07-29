//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPCLASSICALGORITHMBASE_H
#define __INET_TCPCLASSICALGORITHMBASE_H

#include "inet/transportlayer/tcp/flavours/TcpAlgorithmBase.h"
#include "inet/transportlayer/tcp/flavours/TcpClassicAlgorithmBaseState_m.h"

namespace inet {
namespace tcp {

/**
 * Common machinery for the flavours that drive a separate loss-recovery strategy:
 * TcpReno, TcpNewReno, DcTcp and TcpCubic. Besides holding the recovery and
 * congestion-control objects it owns the parts of the ACK path that belong to
 * neither -- duplicate-ACK counting, the Tail Loss Probe outcome, the RFC 3168
 * ECN response -- and forwards send/retransmit/ack events to the recovery.
 *
 * TcpTahoe, TcpVegas, TcpWestwood and TcpNoCongestionControl deliberately stay on
 * TcpAlgorithmBase: they have no recovery strategy to drive.
 */
class INET_API TcpClassicAlgorithmBase : public TcpAlgorithmBase
{
  protected:
    TcpClassicAlgorithmBaseStateVariables *& state; // alias to TcpAlgorithm's 'state'

    ITcpCongestionControl *congestionControl = nullptr;
    ITcpRecovery *recovery = nullptr;

  protected:
    virtual ITcpRecovery *createRecovery() { return nullptr; }
    virtual ITcpCongestionControl *createCongestionControl() { return nullptr; }

    virtual TcpStateVariables *createStateVariables() override
    {
        return new TcpClassicAlgorithmBaseStateVariables();
    }

    virtual void established(bool active) override;

    virtual void processRexmitTimer(TcpEventCode& event) override;

    /** The ssthresh an expired retransmission timer collapses to (RFC 5681 eq. 4). */
    virtual uint32_t calculateSsthreshForRto() { return std::max(getBytesInFlight() / 2, 2 * state->snd_mss); }

    /** The loss window an expired retransmission timer restarts slow start from. */
    virtual uint32_t calculateCwndForRto() { return state->snd_mss; }

    /** Closes out a Tail Loss Probe episode this ACK completed (Linux tcp_process_tlp_ack). */
    virtual void processTlpAck();

    /** RFC 3168 reaction to an ECN-Echo; true if it reduced cwnd on this ACK. */
    virtual bool processEce();

  public:
    /** Ctor */
    TcpClassicAlgorithmBase();
    virtual ~TcpClassicAlgorithmBase();

    virtual void initialize() override;

    virtual ITcpCongestionControl *getCongestionControl() { return congestionControl; }
    virtual ITcpRecovery *getRecovery() override;

    virtual bool isDuplicateAck(const TcpHeader *tcpHeader, uint32_t payloadLength) override;

    virtual void receivedAckForAlreadyAckedData(const TcpHeader *tcpHeader, uint32_t payloadLength) override;

    virtual void receivedAckForUnackedData(uint32_t firstSeqAcked) override;

    virtual void receivedDuplicateAck() override;

    /** Forwarded to the recovery strategy (RFC 6937 PRR accounting, loss probes). */
    virtual void dataSent(uint32_t fromseq) override;

    /** Forwarded to the recovery strategy. */
    virtual void segmentRetransmitted(uint32_t fromseq, uint32_t toseq) override;

    /** Forwarded to the recovery strategy (pre-discard scoreboard inspection). */
    virtual void segmentsAcked(uint32_t fromSeq, uint32_t toSeq) override;

    virtual uint32_t getBytesInFlight() const override;
};

} // namespace tcp
} // namespace inet

#endif

