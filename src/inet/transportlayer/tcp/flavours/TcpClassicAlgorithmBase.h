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
    virtual ITcpCongestionControl *createCongestionControl() = 0;
    virtual ITcpRecovery *createRecovery() = 0;

    /**
     * Whether the ACK of new data ends a fast recovery, so the recovery strategy
     * rather than the congestion control handles it. TcpReno answers from the
     * duplicate-ACK counter, which a timeout does not reset.
     */
    virtual bool isInFastRecovery() const { return state->lossRecovery; }

    /**
     * The ECN-Echo reaction on an ACK of new data. Returns true if it took the
     * place of the window growth. Only TcpReno reacts.
     */
    virtual bool processEce() { return false; }

    /**
     * Called after the window update of an ACK of new data, before the sending:
     * TcpReno continues its SACK-based loss recovery here.
     */
    virtual void ackProcessed(bool inFastRecovery) {}

    /** Redefine what should happen on retransmission */
    virtual void processRexmitTimer(TcpEventCode& event) override;

    /** Called by the base class for each duplicate ACK; the recovery strategy reacts */
    virtual void receivedDuplicateAck() override;

  public:
    /** Ctor */
    TcpClassicAlgorithmBase();

    virtual ~TcpClassicAlgorithmBase();

    virtual void initialize() override;

    virtual void established(bool active) override;

    virtual ITcpRecovery *getRecovery() override { return recovery; }

    /** Redefine what should happen when data got acked, to add congestion window management */
    virtual void receivedAckForUnackedData(uint32_t firstSeqAcked) override;
};


// Deprecated: TcpTahoeRenoFamily was renamed to TcpClassicAlgorithmBase in INET 4.6, because the old name said
// what the class inherits rather than what it is -- it is the base of the classic loss-based algorithms.
// The alias keeps code outside this repository compiling for one release; it goes
// away in the release after that.
using TcpTahoeRenoFamily = TcpClassicAlgorithmBase;

} // namespace tcp
} // namespace inet

#endif

