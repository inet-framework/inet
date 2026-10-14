//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009-2010 Thomas Reschka
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TCPALGORITHM_H
#define __INET_TCPALGORITHM_H

#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/TcpSimsignals.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/tcp/ITcpCongestionControl.h"
#include "inet/transportlayer/tcp/ITcpRecovery.h"

namespace inet {
namespace tcp {

/**
 * Abstract base class for TCP algorithms which encapsulate all behaviour
 * during data transfer state: flavour of congestion control, fast
 * retransmit/recovery, selective acknowledgement etc. Subclasses
 * may implement various sets and flavours of the above algorithms.
 */
class INET_API TcpAlgorithm : public cObject
{
  protected:
    TcpConnection *conn; // we belong to this connection
    TcpStateVariables *state; // our state variables
    simtime_t initialRto; // the RTO before the first RTT measurement (initialRto parameter)
    simtime_t minRexmitTimeout; // the lower limit of the RTO
    simtime_t maxRexmitTimeout; // the upper limit of the RTO
    bool rtoVarianceFloor = false; // the RTO of Linux: the minimum limits the variance term
    bool retainSackAfterRto = false; // a retransmission timeout keeps the SACKed marks, as Linux
    int maxRexmitCount = 0; // the retransmissions of one segment, after which the connection aborts
    simtime_t maxPersistTimeout; // the upper limit of the interval of the zero-window probes
    simtime_t delayedAckTimeout; // the longest delay of an ACK
    bool sendDataWithFirstAck = true; // the data that waits at the end of an active open goes with the ACK of the SYN-ACK

    /**
     * Create state block (TCB) used by this TCP variant. It is expected
     * that every TcpAlgorithm subclass will have its own state block,
     * subclassed from TcpStateVariables. This factory method should
     * create and return a "blank" state block of the appropriate type.
     */
    virtual TcpStateVariables *createStateVariables() = 0;

  public:
    /**
     * Ctor.
     */
    TcpAlgorithm() { state = nullptr; conn = nullptr; }

    /**
     * Virtual dtor.
     */
    virtual ~TcpAlgorithm() {}

    /**
     * Assign this object to a TcpConnection. Its sendQueue and receiveQueue
     * must be set already at this time, because we cache their pointers here.
     */
    void setConnection(TcpConnection *_conn) { conn = _conn; }

    /**
     * Creates and returns the TCP state variables.
     */
    TcpStateVariables *getStateVariables()
    {
        if (!state)
            state = createStateVariables();

        return state;
    }

    /**
     * Should be redefined to initialize the object: create timers, etc.
     * This method is necessary because the TcpConnection ptr is not
     * available in the constructor yet. Subclasses call it first: it reads the
     * timer parameters of the Tcp module.
     */
    virtual void initialize()
    {
        cModule *tcpMain = conn->getTcpMain();
        initialRto = tcpMain->par("initialRto");
        minRexmitTimeout = tcpMain->par("minRexmitTimeout");
        maxRexmitTimeout = tcpMain->par("maxRexmitTimeout");
        rtoVarianceFloor = tcpMain->par("rtoVarianceFloor");
        retainSackAfterRto = tcpMain->par("retainSackAfterRto");
        maxRexmitCount = tcpMain->par("maxRexmitCount");
        maxPersistTimeout = tcpMain->par("maxPersistTimeout");
        delayedAckTimeout = tcpMain->par("delayedAckTimeout");
        sendDataWithFirstAck = tcpMain->par("sendDataWithFirstAck");
    }

    /**
     * Called when the connection is going to ESTABLISHED from SYN_SENT or
     * SYN_RCVD. This is a place to initialize some variables (e.g. set
     * cwnd to the MSS learned during connection setup). If we are on the
     * active side, here we also have to finish the 3-way connection setup
     * procedure by sending an ACK, possibly piggybacked on data.
     */
    virtual void established(bool active) = 0;

    /**
     * Called when the connection closes, it should cancel all running timers.
     */
    virtual void connectionClosed() = 0;

    /**
     * Turns keepalive on or off for this connection (TcpSocket::setKeepAlive()).
     * A flavour without keepalive ignores it.
     */
    virtual void setKeepAlive(bool enabled) {}

    /**
     * Starts the cork timer if it does not run, or cancels it. The timer sends a partial
     * segment that TCP_CORK or MSG_MORE holds after one RTO (Linux ICSK_TIME_PROBE0). A
     * flavour without it does nothing.
     */
    virtual void scheduleCorkTimer() {}
    virtual void cancelCorkTimer() {}

    /**
     * Place to process timers specific to this TcpAlgorithm class.
     * TcpConnection will invoke this method on any timer (self-message)
     * it doesn't recognize (that is, any timer other than the 2MSL,
     * CONN-ESTAB and FIN-WAIT-2 timers).
     *
     * Method may also change the event code (by default set to TCP_E_IGNORE)
     * to cause the state transition of TCP FSM.
     */
    virtual void processTimer(cMessage *timer, TcpEventCode& event) = 0;

    /**
     * Called after user sent TCP_C_SEND command to us.
     */
    virtual void sendCommandInvoked() = 0;

    /**
     * Called after receiving data which are in the window, but not at its
     * left edge (seq != rcv_nxt). This indicates that either segments got
     * re-ordered in the way, or one segment was lost. RFC 1122 and RFC 5681
     * recommend sending an immediate ACK here (Fast Retransmit relies on
     * that).
     */
    virtual void receivedOutOfOrderSegment() = 0;

    /**
     * Called after rcv_nxt got advanced, either because we received in-sequence
     * data ("text" in RFC 9293 lingo) or a FIN. At this point, rcv_nxt has
     * already been updated. This method should take care to send or schedule
     * an ACK some time.
     */
    virtual void receiveSeqChanged() = 0;

    /**
     * Called after we received an ACK for which ackNo <= snd_una.
     */
    virtual void receivedAckForAlreadyAckedData(const TcpHeader *tcpHeader, uint32_t payloadLength) = 0;

    /**
     * Called after we received an ACK which acked some data (that is,
     * we could advance snd_una). At this point the state variables
     * (snd_una, snd_wnd) have already been updated. The argument firstSeqAcked
     * is the previous snd_una value, that is, the number of bytes acked is
     * (snd_una - firstSeqAcked). The dupack counter still reflects the old value
     * (needed for Reno and NewReno); it'll be reset to 0 after this call returns.
     */
    virtual void receivedAckForUnackedData(uint32_t firstSeqAcked) = 0;

    /**
     * Whether this flavour implements SACK-based (RFC 6675) loss recovery.
     * SACK is orthogonal to congestion control (as in Linux): a flavour that
     * returns false will have SACK disabled even if the host is willing, so that
     * turning sackSupport on by default does not break non-SACK flavours.
     */
    virtual bool supportsSackRecovery() const { return false; }

    /**
     * Whether this flavour relies on ECN, as DCTCP (Linux TCP_CONG_NEEDS_ECN): its
     * SYN, SYN-ACK and RST carry ECT (RFC 8257 section 3.6), and the ecnMarkAll
     * parameter applies to its connections.
     */
    virtual bool needsEcn() const { return false; }

    /**
     * Called after we received an ACK for data not yet sent.
     * According to RFC 9293 this function should send an ACK.
     */
    virtual void receivedAckForUnsentData(uint32_t seq) = 0;

    /**
     * Called after we sent an ACK. This hook can be used to cancel
     * the delayed-ACK timer.
     */
    virtual void ackSent() = 0;

    /**
     * Called after we sent data. This hook can be used to schedule the
     * retransmission timer, to start round-trip time measurement, etc.
     * The argument is the seqno of the first byte sent.
     */
    virtual void dataSent(uint32_t fromseq) = 0;

    /**
     * Called after we retransmitted segment.
     * The argument fromseq is the seqno of the first byte sent.
     * The argument toseq is the seqno of the last byte sent+1.
     */
    virtual void segmentRetransmitted(uint32_t fromseq, uint32_t toseq) = 0;

    /**
     * Called when snd_una is about to advance, BEFORE the acked range
     * [fromSeq, toSeq) is discarded from the send/rexmit queues. At this point
     * the scoreboard data for [fromSeq, toSeq) (transmit counts, SACK state) is
     * still valid, so an algorithm can inspect it (e.g. to distinguish reordering
     * from loss). Default-empty; overridden by flavours that need it.
     */
    virtual void segmentsAcked(uint32_t fromSeq, uint32_t toSeq) {}

    /**
     * Called for each ACK at or below snd_max, after the SACK option and before
     * the other ACK processing: the ACK can end the episode of a tail loss probe
     * (RFC 8985 section 7.4.2). Default-empty.
     */
    virtual void processTlpAck(const TcpHeader *tcpHeader, uint32_t payloadLength) {}

    /**
     * Restart REXMIT timer.
     */
    virtual void restartRexmitTimer() = 0;

    /**
     * Converting uint32_t echoedTS to simtime_t and calling rttMeasurementComplete()
     * to update state vars with new measured RTT value.
     */
    virtual void rttMeasurementCompleteUsingTS(uint32_t echoedTS) = 0;

    /**
     * Report a completed RTT measurement (segment sent at tSent, acked at
     * tAcked) to the algorithm's estimator. Used by the connection for the
     * handshake (SYN<->SYN-ACK) RTT seed; data-segment measurements are
     * handled internally by the algorithm.
     */
    virtual void rttMeasurementComplete(simtime_t tSent, simtime_t tAcked) = 0;

    /**
     * Called before sending ACK. Determines whether to set ECE bit.
     */
    virtual bool shouldMarkAck() = 0;

    /**
     * Called before processing segment in established state.
     * This function process ECN marks.
     */
    virtual void processEcnInEstablished() = 0;

    /**
     * Returns the sender's estimation of total bytes in flight in the network.
     */
    virtual uint32_t getBytesInFlight() const = 0;

    /**
     * Returns the smoothed round-trip time, or zero if there is none: DumbTcp has no
     * RTT estimator, and the others have no sample before the first measurement.
     */
    virtual simtime_t getSrtt() const { return SIMTIME_ZERO; }

    /**
     * The new ssthresh when entering fast recovery, per this congestion-control
     * flavour (Linux icsk_ca_ops->ssthresh). Rfc6675Recovery::step4() calls this
     * instead of hardcoding FlightSize/2 so a flavour such as CUBIC can apply its own
     * reduction factor (beta). Non-pure with a 0 default so flavours without a
     * recovery engine (DumbTcp) need no change.
     */
    virtual uint32_t calculateSsthreshForFastRecovery() { return 0; }

    /**
     * The ssthresh this flavour's multiplicative decrease yields for a given flight
     * size (Linux icsk_ca_ops->ssthresh with an explicit argument). Same role as
     * calculateSsthreshForFastRecovery(), for the callers that have already computed
     * the flight size they want the reduction taken from.
     */
    virtual uint32_t calculateSsthresh(uint32_t bytesInFlight) { return 0; }

    /**
     * The connection's loss-recovery strategy, or nullptr for flavours that do
     * not use the ITcpRecovery split (DumbTcp, TcpNoCongestionControl, Vegas,
     * Westwood). Lets the connection reach RACK/PRR/etc. without knowing the
     * concrete algorithm class.
     */
    virtual ITcpRecovery *getRecovery() { return nullptr; }
};

} // namespace tcp
} // namespace inet

#endif

