//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009-2010 Thomas Reschka
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TCPALGORITHMBASE_H
#define __INET_TCPALGORITHMBASE_H

#include "inet/transportlayer/tcp/flavours/TcpAlgorithmBaseState_m.h"
#include "inet/transportlayer/tcp/TcpAlgorithm.h"

namespace inet {
namespace tcp {

/**
 * Includes basic TCP algorithms: adaptive retransmission, PERSIST timer,
 * keep-alive, delayed acks -- EXCLUDING congestion control. Congestion
 * control is implemented in subclasses such as TCPTahoeAlg or TCPRenoAlg.
 *
 * Implements:
 *   - delayed ACK algorithm (RFC 1122)
 *   - Jacobson's and Karn's algorithms for adaptive retransmission
 *   - Nagle's algorithm (RFC 1122) to prevent silly window syndrome
 *   - Increased Initial Window (RFC 3390)
 *   - PERSIST timer
 *
 * To be done:
 *   - KEEP-ALIVE timer
 *
 * Note: currently the timers and time calculations are done in double
 * and NOT in Unix (200ms or 500ms) ticks. It's possible to write another
 * TcpAlgorithm which uses ticks (or rather, factor out timer handling to
 * separate methods, and redefine only those).
 *
 * Congestion window is set to SMSS when the connection is established,
 * and not touched after that. Subclasses may redefine any of the virtual
 * functions here to add their congestion control code.
 */
class INET_API TcpAlgorithmBase : public TcpAlgorithm
{
  protected:
    TcpAlgorithmBaseStateVariables *& state; // alias to TcpAlgorithm's 'state'

    cMessage *rexmitTimer;
    cMessage *persistTimer;
    cMessage *delayedAckTimer;
    cMessage *keepAliveTimer;
    cMessage *tlpTimer; // Tail Loss Probe PTO (RFC 8985 7.2); shares the RTO's single-slot discipline
    cMessage *corkTimer; // TCP_CORK/MSG_MORE flush timer (Linux ICSK_TIME_PROBE0); fires at the RTO

  protected:
    /** @name Process REXMIT, PERSIST, DELAYED-ACK and KEEP-ALIVE timers */
    //@{
    virtual void processRexmitTimer(TcpEventCode& event);
    virtual void processPersistTimer(TcpEventCode& event);
    virtual void processDelayedAckTimer(TcpEventCode& event);
    virtual void processKeepAliveTimer(TcpEventCode& event);
    /** Tail Loss Probe timeout: send a probe and remember snd_max in tlpHighSeq. */
    virtual void processPtoTimer(TcpEventCode& event);
    /** Cork flush timeout: force out the withheld TCP_CORK/MSG_MORE partial with PSH. */
    virtual void processCorkTimer(TcpEventCode& event);
    /** Linux tcp_schedule_loss_probe(): arm the PTO if the connection is TLP-eligible. */
    virtual void schedulePto();
    //@}

    /**
     * Start REXMIT timer and initialize retransmission variables
     */
    virtual void startRexmitTimer();

    /**
     * Re-establish the TCP RTO invariant (Linux tcp_rearm_rto): if any
     * unacknowledged data is outstanding but no retransmission timer is
     * running, arm it. Call after ACK processing has finished sending, to
     * cover data transmitted by RFC 6675 recovery (stepC) or SWS-blocked
     * sends that would otherwise leave outstanding data with no timer.
     */
    void ensureRexmitTimerArmed();

    /**
     * Update state vars with new measured RTT value. Passing two simtime_t's
     * will allow rttMeasurementComplete() to do calculations in double or
     * in 200ms/500ms ticks, as needed)
     */
    virtual void rttMeasurementComplete(simtime_t tSent, simtime_t tAcked) override;

    /**
     * Converting uint32_t echoedTS to simtime_t and calling rttMeasurementComplete()
     * to update state vars with new measured RTT value.
     */
    virtual void rttMeasurementCompleteUsingTS(uint32_t echoedTS) override;

    /**
     * Send data, observing Nagle's algorithm and congestion window
     */
    virtual bool sendData(bool sendCommandInvoked);

    virtual void receivedDuplicateAck();

    /**
     * Returns the configured initial congestion window in bytes according to
     * state->init_cwnd_mode (RFC 2001 / RFC 3390 / RFC 6928 IW10). Used both for
     * the initial cwnd and for the restart window after an idle period.
     */
    virtual uint32_t initialWindow() const;


    /** Utility function */
    cMessage *cancelEvent(cMessage *msg) { return conn->cancelEvent(msg); }

  public:
    /**
     * Ctor.
     */
    TcpAlgorithmBase();

    /**
     * Virtual dtor.
     */
    virtual ~TcpAlgorithmBase();

    /**
     * Create timers, etc.
     */
    virtual void initialize() override;

    virtual void established(bool active) override;

    virtual void connectionClosed() override;

    /**
     * Process REXMIT, PERSIST, DELAYED-ACK and KEEP-ALIVE timers.
     */
    virtual void processTimer(cMessage *timer, TcpEventCode& event) override;

    virtual void sendCommandInvoked() override;

    virtual void scheduleCorkTimer() override;
    virtual void cancelCorkTimer() override;

    virtual void receivedOutOfOrderSegment() override;

    // Linux-shaped adaptive receiver ACK dynamics (adaptiveDelayedAcks param):
    // quickack budget, adaptive delayed-ACK timeout (ATO), pingpong mode
    virtual void incrQuickack(uint32_t maxQuickacks);
    virtual void enterQuickackMode(uint32_t maxQuickacks);
    virtual bool inQuickackMode() const;
    virtual void dataArrivedAtoUpdate();
    virtual void scheduleDelayedAck();

    virtual void receiveSeqChanged() override;

    virtual void receivedAckForAlreadyAckedData(const TcpHeader *tcpHeader, uint32_t payloadLength) override;

    virtual void receivedAckForUnackedData(uint32_t firstSeqAcked) override;

    virtual void receivedAckForUnsentData(uint32_t seq) override;

    virtual void ackSent() override;

    virtual void dataSent(uint32_t fromseq) override;

    virtual void segmentRetransmitted(uint32_t fromseq, uint32_t toseq) override;

    virtual void restartRexmitTimer() override;

    virtual bool shouldMarkAck() override;

    virtual void processEcnInEstablished() override;

    virtual uint32_t getBytesInFlight() const override;

    virtual simtime_t getSrtt() const override { return state->srtt; }

    virtual uint32_t calculateSsthreshForFastRecovery() override;
};

} // namespace tcp
} // namespace inet

#endif

