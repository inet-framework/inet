//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009-2010 Thomas Reschka
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TCPBASEALG_H
#define __INET_TCPBASEALG_H

#include "inet/transportlayer/tcp/TcpAlgorithm.h"
#include "inet/transportlayer/tcp/flavours/TcpAlgorithmBaseState_m.h"
#include "inet/transportlayer/tcp/TcpSimsignals.h"

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
 *   - Nagle's algorithm (RFC 896) to prevent silly window syndrome
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

  protected:
    /** @name Process REXMIT, PERSIST, DELAYED-ACK and KEEP-ALIVE timers */
    //@{
    virtual void processRexmitTimer(TcpEventCode& event);
    virtual void processPersistTimer(TcpEventCode& event);
    virtual void processDelayedAckTimer(TcpEventCode& event);
    virtual void processKeepAliveTimer(TcpEventCode& event);
    //@}

    /**
     * Start REXMIT timer and initialize retransmission variables
     */
    virtual void startRexmitTimer();

    /**
     * Re-establish the TCP RTO invariant (Linux tcp_rearm_rto): if any
     * unacknowledged data is outstanding but no retransmission timer is
     * running, arm it. Call after ACK processing has finished sending, to
     * cover data transmitted by RFC 6675 recovery (stepC), which does not
     * arm the timer.
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
     * Called after we received a duplicate ACK (that is: ackNo == snd_una,
     * no data in segment, and also, we have unacked data). The dupack counter
     * got already updated when calling this method (i.e. dupacks == 1 on the
     * first duplicate ACK.)
     */
    virtual void receivedDuplicateAck();

    /** Utility function */
    cMessage *cancelEvent(cMessage *msg) { return conn->cancelEvent(msg); }

  public:
    /**
     * Send data, observing Nagle's algorithm and congestion window. Public,
     * because the recovery strategies of the classic flavours send through it.
     */
    virtual bool sendData(bool sendCommandInvoked);

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

    virtual void receivedOutOfOrderSegment() override;

    virtual void receiveSeqChanged() override;

    virtual void receivedAckForAlreadyAckedData(const TcpHeader *tcpHeader, uint32_t payloadLength) override;

    /** The duplicate-ACK test: ackNo == snd_una, no data, and unacked data. */
    virtual bool isDuplicateAck(const TcpHeader *tcpHeader, uint32_t payloadLength);

    /** Maintains state->dupacks and dispatches receivedDuplicateAck(). */
    virtual void countDuplicateAck(const TcpHeader *tcpHeader, uint32_t payloadLength);

    virtual void receivedAckForUnackedData(uint32_t firstSeqAcked) override;

    virtual void receivedAckForUnsentData(uint32_t seq) override;

    virtual void ackSent() override;

    virtual void dataSent(uint32_t fromseq) override;

    virtual void segmentRetransmitted(uint32_t fromseq, uint32_t toseq) override;

    virtual void restartRexmitTimer() override;

    virtual bool shouldMarkAck() override;

    virtual void processEcnInEstablished() override;
    virtual uint32_t getBytesInFlight() const override;
    virtual uint32_t calculateSsthresh(uint32_t bytesInFlight) override;

    virtual uint32_t calculateSsthreshForFastRecovery() override;
};


// Deprecated: TcpBaseAlg was renamed to TcpAlgorithmBase in INET 4.6, because the old name said
// what the class inherits rather than what it is -- it is the base of every TCP algorithm.
// The alias keeps code outside this repository compiling for one release; it goes
// away in the release after that.
using TcpBaseAlg = TcpAlgorithmBase;

} // namespace tcp
} // namespace inet

#endif

