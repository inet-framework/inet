//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009-2010 Thomas Reschka
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/TcpAlgorithmBase.h"

#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"

namespace inet {
namespace tcp {

// RFC 1122, page 95:
// "A TCP SHOULD implement a delayed ACK, but an ACK should not
// be excessively delayed; in particular, the delay MUST be
// less than 0.5 seconds, and in a stream of full-sized
// segments there SHOULD be an ACK for at least every second
// segment."

std::string TcpAlgorithmBaseStateVariables::str() const
// Linux-shaped adaptive receiver ACK dynamics (adaptiveDelayedAcks parameter);
// values are Linux's long-stable ABI constants (TCP_ATO_MIN/TCP_DELACK_MIN =
// HZ/25, TCP_DELACK_MAX = HZ/5, TCP_MAX_QUICKACKS, TCP_PINGPONG_THRESH).
#define TCP_ATO_MIN_S          0.04  // 40ms: ATO floor and quickack-mode ATO
#define TCP_DELACK_MIN_S       0.04  // 40ms
#define TCP_DELACK_MAX_S       0.2   // 200ms
#define TCP_MAX_QUICKACKS      16
#define TCP_PINGPONG_THRESH    1 // Linux sysctl_tcp_pingpong_thresh default (tcp_ipv4.c)
{
    std::stringstream out;
    out << TcpStateVariables::str();
    out << " snd_cwnd=" << snd_cwnd;
    out << " rto=" << rexmit_timeout;
    return out.str();
}

std::string TcpAlgorithmBaseStateVariables::detailedInfo() const
{
    std::stringstream out;
    out << TcpStateVariables::detailedInfo();
    out << "snd_cwnd=" << snd_cwnd << "\n";
    out << "rto=" << rexmit_timeout << "\n";
    out << "persist_timeout=" << persist_timeout << "\n";
    // TODO add others too
    return out.str();
}

TcpAlgorithmBase::TcpAlgorithmBase() : TcpAlgorithm(),
    state((TcpAlgorithmBaseStateVariables *&)TcpAlgorithm::state)
{
    rexmitTimer = persistTimer = delayedAckTimer = keepAliveTimer = tlpTimer = corkTimer = nullptr;
}

TcpAlgorithmBase::~TcpAlgorithmBase()
{
    // Note: don't delete "state" here, it'll be deleted from TcpConnection
    // cancel and delete timers
    if (rexmitTimer)
        delete cancelEvent(rexmitTimer);
    if (persistTimer)
        delete cancelEvent(persistTimer);
    if (delayedAckTimer)
        delete cancelEvent(delayedAckTimer);
    if (keepAliveTimer)
        delete cancelEvent(keepAliveTimer);
    if (tlpTimer)
        delete cancelEvent(tlpTimer);
    if (corkTimer)
        delete cancelEvent(corkTimer);
}

void TcpAlgorithmBase::initialize()
{
    TcpAlgorithm::initialize();

    rexmitTimer = new cMessage("REXMIT");
    persistTimer = new cMessage("PERSIST");
    delayedAckTimer = new cMessage("DELAYEDACK");
    keepAliveTimer = new cMessage("KEEPALIVE");
    tlpTimer = new cMessage("TLP-PTO");
    // schedulePto() caps the probe at the RTO's remaining time, so the two
    // timers can land on the very same instant; Linux keeps them in ONE icsk
    // slot where an armed probe REPLACES the RTO. Give the probe the earlier
    // position on ties -- its handler then pushes the RTO out by a full
    // period (rearm in processPtoTimer), while a failed/skipped probe leaves
    // the same-instant RTO to fire right after, so no deadlock is possible.
    tlpTimer->setSchedulingPriority(-1);
    corkTimer = new cMessage("CORK");

    rexmitTimer->setContextPointer(conn);
    persistTimer->setContextPointer(conn);
    delayedAckTimer->setContextPointer(conn);
    keepAliveTimer->setContextPointer(conn);
    tlpTimer->setContextPointer(conn);
    corkTimer->setContextPointer(conn);

    state->keepalive_enabled = conn->getTcpMain()->par("keepAliveEnabled");
    state->keepalive_idle_time = conn->getTcpMain()->par("keepAliveIdleTime");
    state->keepalive_interval = conn->getTcpMain()->par("keepAliveInterval");
    state->keepalive_max_probes = conn->getTcpMain()->par("keepAliveProbeCount");

    state->rexmit_timeout = initialRto;
}

uint32_t TcpAlgorithmBase::initialWindow() const
{
    // A route's initcwnd outranks the RFC default, as it does in Linux
    // (dst_metric(RTAX_INITCWND) wins over TCP_INIT_CWND in tcp_init_cwnd).
    if (state->initialCwndSegments > 0)
        return state->initialCwndSegments * state->snd_effmss;

    switch (state->init_cwnd_mode) {
        case 1: // RFC 3390
            return std::min(4 * state->snd_effmss, std::max(2 * state->snd_effmss, (uint32_t)4380));
        case 2: // RFC 6928 (IW10)
            return std::min(10 * state->snd_effmss, std::max(2 * state->snd_effmss, (uint32_t)14600));
        default: // RFC 2001: one segment
            return state->snd_effmss;
    }
}

void TcpAlgorithmBase::established(bool active)
{
    // Linux seeds icsk_ack.lrcvtime at connection establishment
    // (tcp_finish_connect / openreq child init), NOT at the first data
    // arrival: a data segment sent within one ATO of the handshake already
    // counts as interactive ("pingpong") evidence, suppressing the quickack
    // that would otherwise ACK the peer's reply immediately
    // (fastopen cookie-less-sendto pins the reply data being ACKed only by
    // the subsequent close()'s FIN).
    state->lastDataRecvTime = simTime();

    // Linux tcp_rcv_synsent_state_process, write-pending arm: when data is
    // already queued behind the handshake (a TFO remainder, a deferred
    // send), the bare third ACK is saved ("data will be ready after several
    // ticks") and tcp_enter_quickack_mode() runs -- seeding the ATO, so the
    // data leaving this very instant registers as pingpong evidence in
    // dataSent(). The peer's first reply is then ACKed on the DELAYED path
    // (cookie-less-sendto: reply data acked only by the close()'s FIN).
    if (active && state->adaptiveDelayedAcks
            && conn->getSendQueue()->getBytesAvailable(state->snd_nxt) > 0)
        enterQuickackMode(TCP_MAX_QUICKACKS);

    // "Prevent spurious tcp_cwnd_restart() on first data" (tcp_finish_connect):
    // a slow handshake (e.g. a retransmitted TFO SYN, +1s) must not count as
    // idle time -- without this, the after-idle restart clamps cwnd right when
    // the unacknowledged SYN data is being retransmitted and the send stalls
    // (cookie-less-sendto's non-blocking test pins P. 1:1001 leaving WITH the
    // handshake ACK).
    state->time_last_data_sent = simTime();

    // initialize cwnd (we may learn SMSS during connection setup)

    // RFC 3390, page 2: "The upper bound for the initial window is given more precisely in
    // (1):
    //
    //   min (4*MSS, max (2*MSS, 4380 bytes))                        (1)
    //
    // Note: Sending a 1500 byte packet indicates a maximum segment size
    // (MSS) of 1460 bytes (assuming no IP or TCP options).  Therefore,
    // limiting the initial window's MSS to 4380 bytes allows the sender to
    // transmit three segments initially in the common case when using 1500
    // byte packets.
    //
    // Equivalently, the upper bound for the initial window size is based on
    // the MSS, as follows:
    //
    //   If (MSS <= 1095 bytes)
    //     then win <= 4 * MSS;
    //   If (1095 bytes < MSS < 2190 bytes)
    //     then win <= 4380;
    //   If (2190 bytes <= MSS)
    //     then win <= 2 * MSS;
    //
    // This increased initial window is optional: a TCP MAY start with a
    // larger initial window.  However, we expect that most general-purpose
    // TCP implementations would choose to use the larger initial congestion
    // window given in equation (1) above.
    //
    // This upper bound for the initial window size represents a change from
    // RFC 2581 [RFC 2581], which specified that the congestion window be
    // initialized to one or two segments.
    // (...)
    // If the SYN or SYN/ACK is
    // lost, the initial window used by a sender after a correctly
    // transmitted SYN MUST be one segment consisting of MSS bytes."
    // RFC 3390/6928: if the SYN or SYN/ACK was lost, the initial window is 1 SMSS.
    if (state->syn_rexmit_count == 0) {
        state->snd_cwnd = initialWindow();
        if (state->init_cwnd_mode != 0)
            EV_DETAIL << "Increased Initial Window, CWND is set to " << state->snd_cwnd << "\n";
    }
    else
        state->snd_cwnd = state->snd_effmss;

    // TODO we should send the ACK from TcpConnection instead of TcpAlgorithmBase, this is standard TCP behavior
    if (active) {
        // finish connection setup with ACK (possibly piggybacked on data)
        EV_INFO << "Completing connection setup by sending ACK (possibly piggybacked on data)\n";
        if (sendDataWithFirstAck) {
            if (!sendData(false))
                conn->sendAck();
        }
        else {
            conn->sendAck();
            sendData(false);
        }
    }

    if (state->keepalive_enabled) {
        state->time_last_segment_received = simTime();
        state->keepalive_probes_sent = 0;
        conn->scheduleAfter(state->keepalive_idle_time, keepAliveTimer);
    }
}

void TcpAlgorithmBase::connectionClosed()
{
    cancelEvent(rexmitTimer);
    cancelEvent(persistTimer);
    cancelEvent(delayedAckTimer);
    cancelEvent(keepAliveTimer);
    cancelEvent(tlpTimer);
    cancelEvent(corkTimer);
}

void TcpAlgorithmBase::processTimer(cMessage *timer, TcpEventCode& event)
{
    if (timer == rexmitTimer)
        processRexmitTimer(event);
    else if (timer == persistTimer)
        processPersistTimer(event);
    else if (timer == delayedAckTimer)
        processDelayedAckTimer(event);
    else if (timer == keepAliveTimer)
        processKeepAliveTimer(event);
    else if (timer == tlpTimer)
        processPtoTimer(event);
    else if (timer == corkTimer)
        processCorkTimer(event);
    else
        throw cRuntimeError(timer, "unrecognized timer");
}

void TcpAlgorithmBase::processCorkTimer(TcpEventCode& event)
{
    // Linux ICSK_TIME_PROBE0 fired for a corked partial: force it out with PSH
    // (tcp_write_wakeup forces PSH). corkedDataPending is cleared by the flush.
    conn->flushCorkedData(/*forcePush=*/true);
}

void TcpAlgorithmBase::scheduleCorkTimer()
{
    // Force-flush a withheld TCP_CORK/MSG_MORE partial at the RTO if nothing else
    // (a later send, an incoming ACK, or an uncork) flushes it first.
    if (corkTimer->isScheduled())
        conn->cancelEvent(corkTimer);
    conn->scheduleAfter(state->rexmit_timeout, corkTimer);
}

void TcpAlgorithmBase::cancelCorkTimer()
{
    if (corkTimer != nullptr && corkTimer->isScheduled())
        conn->cancelEvent(corkTimer);
}

void TcpAlgorithmBase::schedulePto()
{
    // Linux tcp_schedule_loss_probe(): eligible while SACK-capable, not in loss
    // recovery, with no SACKed data outstanding, and no probe already in flight.
    if (!state->tlpEnabled || !state->sack_enabled || state->lossRecovery
            || state->sackedBytes != 0 || state->tlpHighSeq != 0
            || state->snd_una == state->snd_max)
        return;

    // PTO = 2*SRTT; with a single packet in flight add the peer's potential
    // delayed-ACK wait. No RTT sample yet -> the current RTO.
    simtime_t pto;
    if (state->srtt > 0) {
        pto = state->srtt * 2;
        if (state->snd_max - state->snd_una <= state->snd_mss)
            pto += minRexmitTimeout; // single packet in flight: allow for the peer's delayed ACK
        else
            pto += SimTime(2, SIMTIME_MS); // floor so a near-zero srtt cannot fire the
                                           // probe between back-to-back ACKs of one flight
    }
    else
        pto = state->rexmit_timeout;

    // never fire later than the RTO would have
    if (rexmitTimer->isScheduled()) {
        simtime_t rtoRemaining = rexmitTimer->getArrivalTime() - simTime();
        if (rtoRemaining < pto)
            pto = rtoRemaining;
    }
    if (pto <= SIMTIME_ZERO)
        return;

    if (tlpTimer->isScheduled())
        conn->cancelEvent(tlpTimer);
    conn->scheduleAfter(pto, tlpTimer);
    EV_DETAIL << "TLP: probe timeout armed for " << pto << "s\n";
}

void TcpAlgorithmBase::processPtoTimer(TcpEventCode& event)
{
    // RFC 8985 section 7.2 / Linux tcp_send_loss_probe(): the tail of the flight
    // was not acked within the probe timeout. Send one probe segment so its ACK
    // (or the SACK hole it exposes) triggers fast recovery instead of an RTO.
    //
    // Not while already in fast recovery: Linux shares the RETRANS/LOSS_PROBE icsk
    // timer slot, so entering recovery arms the RTO and supersedes the PTO. INET's
    // recovery can be entered by the RACK reordering timer WITHOUT restarting the RTO
    // (a pure-SACK recovery advances no cumulative ACK), leaving a stale PTO armed; it
    // must not fire a redundant last-segment probe once RACK/PRR own recovery.
    if (!state->tlpEnabled || state->tlpHighSeq != 0 || state->lossRecovery)
        return;
    if (conn->sendTlpProbe()) {
        state->tlpHighSeq = state->snd_max; // probe outstanding until this is acked
        // Single timer slot, the other direction: a fired probe supersedes the
        // pending RTO and re-arms it for a full period from now (Linux
        // tcp_send_loss_probe's rearm_timer). schedulePto() caps the PTO at the
        // RTO's remaining time, so both can be scheduled for the very same
        // instant -- without this the RTO event still fires right after the
        // probe and retransmits the head a full RTO period early.
        if (rexmitTimer->isScheduled())
            conn->cancelEvent(rexmitTimer);
        conn->scheduleAfter(state->rexmit_timeout, rexmitTimer);
    }
}

void TcpAlgorithmBase::processRexmitTimer(TcpEventCode& event)
{
    EV_DETAIL << "TCB: " << state->str() << "\n";

    //"
    // For any state if the retransmission timeout expires on a segment in
    // the retransmission queue, send the segment at the front of the
    // retransmission queue again, reinitialize the retransmission timer,
    // and return.
    //"
    // Also: abort connection after max 12 retries.
    //
    // However, retransmission is actually more complicated than that
    // in RFC 9293 above, we'll leave it to subclasses (e.g. TcpTahoe, TcpReno).
    //
    if (++state->rexmit_count > maxRexmitCount) {
        EV_DETAIL << "Retransmission count exceeds " << maxRexmitCount << ", aborting connection\n";
        conn->signalConnectionTimeout();
        event = TCP_E_ABORT; // TODO maybe rather introduce a TCP_E_TIMEDOUT event
        return;
    }

    EV_INFO << "Performing retransmission #" << state->rexmit_count
            << "; increasing RTO from " << state->rexmit_timeout << "s ";

    //
    // Karn's algorithm is implemented below:
    //  (1) don't measure RTT for retransmitted packets.
    //  (2) RTO should be doubled after retransmission ("exponential back-off")
    //

    // restart the retransmission timer with twice the latest RTO value, or with the max, whichever is smaller
    state->rexmit_timeout += state->rexmit_timeout;
    if (state->rexmit_timeout > maxRexmitTimeout)
        state->rexmit_timeout = maxRexmitTimeout;

    conn->scheduleAfter(state->rexmit_timeout, rexmitTimer);

    // Single timer slot (Linux shares the RETRANS and LOSS_PROBE slot): a fired
    // RTO supersedes any pending loss probe. schedulePto() caps the PTO at the
    // RTO's remaining time, so the two can be scheduled for the very same instant;
    // without this cancel the RTO fires, retransmits, and then a stale TLP probe
    // fires into the post-RTO state (a spurious extra segment, and -- before the
    // sendTlpProbe bound fix -- a createSegmentWithBytes abort).
    if (tlpTimer != nullptr && tlpTimer->isScheduled())
        conn->cancelEvent(tlpTimer);

    EV_INFO << " to " << state->rexmit_timeout << "s, and cancelling RTT measurement\n";

    // cancel round-trip time measurement
    state->rtseq_sendtime = 0;

    state->numRtos++;

    conn->emit(numRtosSignal, state->numRtos);

    // if sacked_enabled reset sack related flags
    if (state->sack_enabled) {
        conn->getRexmitQueueForUpdate()->resetSackedBit();
        conn->getRexmitQueueForUpdate()->resetRexmittedBit();

        // RFC 6675, page 10: "If an RTO occurs during loss recovery as specified in this document,
        // RecoveryPoint MUST be set to HighData.  Further, the new value of
        // RecoveryPoint MUST be preserved and the loss recovery algorithm
        // outlined in this document MUST be terminated.  In addition, a new
        // recovery phase (as described in section 5) MUST NOT be initiated
        // until HighACK is greater than or equal to the new value of
        // RecoveryPoint."
        if (state->lossRecovery) {
            state->recoveryPoint = state->snd_max; // HighData = snd_max
            EV_DETAIL << "Loss Recovery terminated.\n";
            state->lossRecovery = false;
        }
    }

    state->time_last_data_sent = simTime();

    //
    // Leave congestion window management and actual retransmission to
    // subclasses (e.g. TcpTahoe, TcpReno).
    //
    // That is, subclasses will redefine this method, call us, then perform
    // window adjustments and do the retransmission as they like.
    //
}

void TcpAlgorithmBase::processPersistTimer(TcpEventCode& event)
{
    // Linux tcp_probe_timer / probe0 cadence (resolves the old FIXME): the
    // zero-window probe interval starts at the current RTO (set at first arm,
    // see receivedAckForUnackedData's zero-window branch) and DOUBLES per
    // probe (icsk_backoff), capped at maxPersistTimeout -- it is not the
    // fixed Stevens 5/5/6/12/24/48/60 table: with a ~100ms RTT the first
    // probe goes out at ~300ms (slow-start-after-win-update pins this).
    state->persist_timeout = state->persist_timeout * 2;
    if (state->persist_timeout > maxPersistTimeout)
        state->persist_timeout = maxPersistTimeout;

    conn->scheduleAfter(state->persist_timeout, persistTimer);

    // sending persist probe
    conn->sendProbe();
    state->zeroWindowProbesSent++;
}

void TcpAlgorithmBase::processDelayedAckTimer(TcpEventCode& event)
{
    if (state->adaptiveDelayedAcks) {
        // a delayed ACK actually expired (Linux tcp_delack_timer_handler):
        // in bulk mode the ATO was too optimistic -- inflate it (bounded by
        // RTO); in interactive (pingpong) mode drop back out and deflate
        if (state->pingpongCount < TCP_PINGPONG_THRESH) {
            state->ackAto = state->ackAto * 2;
            if (state->ackAto > state->rexmit_timeout)
                state->ackAto = state->rexmit_timeout;
        }
        else {
            state->pingpongCount = 0;
            state->ackAto = TCP_ATO_MIN_S;
        }
    }
    state->ack_now = true;
    conn->sendAck();
}

void TcpAlgorithmBase::processKeepAliveTimer(TcpEventCode& event)
{
    // RFC 1122 4.2.3.6 keepalive mechanism, following the Linux tcp_keepalive_timer
    // semantics (net/ipv4/tcp_timer.c).

    // If there is unacknowledged data or data pending in the send queue, the
    // retransmission timer already probes connection liveness; just re-arm.
    if (state->snd_max != state->snd_una || !conn->isSendQueueEmpty()) {
        state->keepalive_probes_sent = 0;
        conn->scheduleAfter(state->keepalive_idle_time, keepAliveTimer);
        return;
    }

    // If a segment was received recently, the connection is not idle yet.
    simtime_t elapsed = simTime() - state->time_last_segment_received;
    if (elapsed < state->keepalive_idle_time) {
        state->keepalive_probes_sent = 0;
        conn->scheduleAfter(state->keepalive_idle_time - elapsed, keepAliveTimer);
        return;
    }

    // The connection is idle. If the peer failed to answer the allowed number of
    // probes, abort the connection (Linux sends a RST; INET reuses the
    // timeout-abort path, which notifies the app with TCP_I_TIMED_OUT).
    if (state->keepalive_probes_sent >= state->keepalive_max_probes) {
        EV_INFO << "Keepalive: peer did not respond to " << state->keepalive_max_probes
                << " probes, aborting connection\n";
        conn->signalConnectionTimeout();
        event = TCP_E_ABORT;
        return;
    }

    EV_INFO << "Keepalive: connection idle, sending probe #"
            << (state->keepalive_probes_sent + 1) << "\n";
    conn->sendKeepAliveProbe();
    state->keepalive_probes_sent++;
    conn->scheduleAfter(state->keepalive_interval, keepAliveTimer);
}

void TcpAlgorithmBase::startRexmitTimer()
{
    // start counting retransmissions for this seq number.
    // Note: state->rexmit_timeout is set from rttMeasurementComplete().
    state->rexmit_count = 0;

    // single-slot discipline with the loss-probe timer (Linux shares one icsk
    // timer slot between RETRANS and LOSS_PROBE): arming the RTO always disarms
    // a pending probe.
    if (tlpTimer != nullptr && tlpTimer->isScheduled())
        conn->cancelEvent(tlpTimer);

    // schedule timer
    conn->scheduleAfter(state->rexmit_timeout, rexmitTimer);
}

void TcpAlgorithmBase::ensureRexmitTimerArmed()
{
    // TCP RTO invariant (Linux tcp_rearm_rto): unacknowledged data outstanding
    // implies the retransmission timer must be running. receivedAckForUnackedData
    // cancels the timer on an ACK that acks all previously-outstanding data, but
    // RFC 6675 recovery (stepC) can then transmit fresh segments in the same ACK
    // whose send path does not arm the timer, and the trailing sendData() may be
    // cwnd-blocked (SWS) and send nothing. Re-arm here so that fresh data cannot
    // be left outstanding with no timer -- otherwise, if it is lost, nothing ever
    // retransmits it and the connection deadlocks.
    if (state->snd_una != state->snd_max && !rexmitTimer->isScheduled())
        startRexmitTimer();
}

void TcpAlgorithmBase::rttMeasurementComplete(simtime_t tSent, simtime_t tAcked)
{
    //
    // Jacobson's algorithm for estimating RTT and adaptively setting RTO.
    //
    // Note: this implementation calculates in doubles. An impl. which uses
    // 500ms ticks is available from old tcpmodule.cc:calcRetransTimer().
    //

    // RTT estimator per RFC 6298 (Jacobson/Karn), with Linux's variance-floor RTO.
    // update smoothed RTT estimate (srtt) and variance (rttvar)
    const double g = 0.125; // 1 / 8; (1 - alpha) where alpha == 7 / 8;
    simtime_t newRTT = tAcked - tSent;

    // track the minimum RTT (RACK loss detection); a running min (not windowed)
    if (newRTT > 0 && (state->minRtt == 0 || newRTT < state->minRtt))
        state->minRtt = newRTT;

    simtime_t err = newRTT - state->srtt;

    if (state->srtt == 0) {
        state->srtt = newRTT;
        state->rttvar = newRTT / 2;
    }
    else {
        state->srtt += g * err;
        state->rttvar += g * (fabs(err) - state->rttvar);
    }

    // Linux-style variance floor (tcp_set_rto): RTO = SRTT + max(4*RTTVAR, RTO_MIN),
    // i.e. RTO >= SRTT + minRexmitTimeout, rather than clamping the final RTO from
    // below.
    simtime_t varTerm = 4 * state->rttvar;
    if (varTerm < minRexmitTimeout)
        varTerm = minRexmitTimeout;
    simtime_t rto = state->srtt + varTerm;

    if (rto > maxRexmitTimeout)
        rto = maxRexmitTimeout;

    state->rexmit_timeout = rto;

    // record statistics
    EV_DETAIL << "Measured RTT=" << (newRTT * 1000) << "ms, updated SRTT=" << (state->srtt * 1000)
              << "ms, new RTO=" << (rto * 1000) << "ms\n";

    conn->emit(rttSignal, newRTT);
    conn->emit(srttSignal, state->srtt);
    conn->emit(rttvarSignal, state->rttvar);
    conn->emit(rtoSignal, rto);
}

void TcpAlgorithmBase::rttMeasurementCompleteUsingTS(uint32_t echoedTS)
{
    ASSERT(state->ts_enabled);

    // Note: The TS option is using uint32_t values (ms precision) therefore we convert the current simTime also to a uint32_t value (ms precision)
    // and then convert back to simtime_t to use rttMeasurementComplete() to update srtt and rttvar
    uint32_t now = conn->convertSimtimeToTS(simTime());
    simtime_t tSent = conn->convertTSToSimtime(echoedTS);
    simtime_t tAcked = conn->convertTSToSimtime(now);
    rttMeasurementComplete(tSent, tAcked);
}

bool TcpAlgorithmBase::sendData(bool sendCommandInvoked)
{
    // TCP Fast Open server (RFC 7413 section 4.2): response data may be sent
    // from SYN_RCVD, before established() has initialized the congestion
    // window -- initialize it here, the same initial window a regular
    // connection would get (Linux initializes the TFO child socket's cwnd at
    // creation). Only a fastopenAccelerated connection can reach
    // sendData() with the pre-established cwnd of 0.
    if (state->snd_cwnd == 0 && state->fastopenAccelerated) {
        state->snd_cwnd = initialWindow();
        EV_DETAIL << "Fast Open: initializing CWND to " << state->snd_cwnd << " for SYN_RCVD response data\n";
    }

    // RFC 5681, page 11: "When TCP has not received a segment for
    // more than one retransmission timeout, cwnd is reduced to the value
    // of the restart window (RW) before transmission begins.
    // For the purposes of this standard, we define RW = IW.
    // (...)
    // Using the last time a segment was received to determine whether or
    // not to decrease cwnd fails to deflate cwnd in the common case of
    // persistent HTTP connections [HTH98].
    // (...)
    // Therefore, a TCP SHOULD set cwnd to no more than RW before beginning
    // transmission if the TCP has not sent data in an interval exceeding
    // the retransmission timeout."
    if (!conn->isSendQueueEmpty()) { // do we have any data to send?
        if ((simTime() - state->time_last_data_sent) > state->rexmit_timeout) {
            // RFC 5681, page 11: "For the purposes of this standard, we define RW = min(IW,cwnd)."
            state->snd_cwnd = std::min(initialWindow(), state->snd_cwnd);

            EV_INFO << "Restarting idle connection, CWND is set to " << state->snd_cwnd << "\n";
        }
    }

    //
    // Send window is effectively the minimum of the congestion window (cwnd)
    // and the advertised window (snd_wnd).
    //
    return conn->sendData(state->snd_cwnd);
}

void TcpAlgorithmBase::sendCommandInvoked()
{
    // try sending
    sendData(true);
}

void TcpAlgorithmBase::incrQuickack(uint32_t maxQuickacks)
{
    // Budget of back-to-back immediate ACKs: enough to cover half the receive
    // window in one-per-segment ACKs, at most maxQuickacks (Linux
    // tcp_incr_quickack; rcv_mss approximated by our own MSS -- the corpus
    // and virtually all sim setups are MSS-symmetric).
    uint32_t mss = state->snd_mss > 0 ? state->snd_mss : 536;
    uint32_t quickacks = state->rcv_wnd / (2 * mss);
    if (quickacks == 0)
        quickacks = 2;
    if (quickacks > maxQuickacks)
        quickacks = maxQuickacks;
    if (quickacks > state->quickAckCounter)
        state->quickAckCounter = quickacks;
}

void TcpAlgorithmBase::enterQuickackMode(uint32_t maxQuickacks)
{
    incrQuickack(maxQuickacks);
    state->pingpongCount = 0; // leave interactive mode
    state->ackAto = TCP_ATO_MIN_S;
}

bool TcpAlgorithmBase::inQuickackMode() const
{
    return state->quickAckCounter > 0 && state->pingpongCount < TCP_PINGPONG_THRESH;
}

void TcpAlgorithmBase::receivedOutOfOrderSegment()
{
    // out-of-order data starts (or refreshes) a quickack burst: the sender is
    // likely in loss recovery and needs feedback per segment
    if (state->adaptiveDelayedAcks)
        enterQuickackMode(TCP_MAX_QUICKACKS);
    state->ack_now = true;
    EV_INFO << "Out-of-order segment, sending immediate ACK\n";
    conn->sendAck();
}

void TcpAlgorithmBase::dataArrivedAtoUpdate()
{
    // Adapt the delayed-ACK engine to the observed inter-segment arrival gap
    // (Linux tcp_event_data_recv): the first data segment initializes a full
    // quickack budget; closely spaced arrivals shrink the ATO toward its
    // 40ms floor; a gap above the retransmission timeout means the sender
    // stalled waiting for ACKs -- resume quick ACKing.
    simtime_t now = simTime();
    if (state->ackAto == SIMTIME_ZERO) {
        incrQuickack(TCP_MAX_QUICKACKS);
        state->ackAto = TCP_ATO_MIN_S;
    }
    else {
        simtime_t m = now - state->lastDataRecvTime;
        if (m <= TCP_ATO_MIN_S / 2)
            state->ackAto = state->ackAto / 2 + TCP_ATO_MIN_S / 2;
        else if (m < state->ackAto) {
            state->ackAto = state->ackAto / 2 + m;
            if (state->ackAto > state->rexmit_timeout)
                state->ackAto = state->rexmit_timeout;
        }
        else if (m > state->rexmit_timeout)
            incrQuickack(TCP_MAX_QUICKACKS);
    }
    state->lastDataRecvTime = now;
}

void TcpAlgorithmBase::scheduleDelayedAck()
{
    // Linux tcp_send_delayed_ack: the armed timeout is the ATO bounded by the
    // measured RTT (a delayed ACK should not stall the sender's clock for
    // longer than a round trip) and by the 200ms ceiling.
    simtime_t ato = state->ackAto;
    if (ato > TCP_DELACK_MIN_S) {
        simtime_t maxAto = TCP_DELACK_MAX_S;
        if (state->srtt > SIMTIME_ZERO) {
            simtime_t rtt = state->srtt < TCP_DELACK_MIN_S ? TCP_DELACK_MIN_S : state->srtt;
            if (rtt < maxAto)
                maxAto = rtt;
        }
        if (ato > maxAto)
            ato = maxAto;
    }
    if (ato > TCP_DELACK_MAX_S)
        ato = TCP_DELACK_MAX_S;

    simtime_t timeout = simTime() + ato;
    if (delayedAckTimer->isScheduled()) {
        // an earlier deadline stands; and if it is about to fire anyway,
        // just send the ACK now
        if (delayedAckTimer->getArrivalTime() <= simTime() + ato / 4) {
            cancelEvent(delayedAckTimer);
            state->ack_now = true;
            conn->sendAck();
            return;
        }
        if (delayedAckTimer->getArrivalTime() < timeout)
            return; // keep the earlier one
        cancelEvent(delayedAckTimer);
    }
    conn->scheduleAt(timeout, delayedAckTimer);
}

void TcpAlgorithmBase::receiveSeqChanged()
{
    // If we send a data segment already (with the updated seqNo) there is no need to send an additional ACK
    if (state->full_sized_segment_counter == 0 && !state->ack_now && state->last_ack_sent == state->rcv_nxt && !delayedAckTimer->isScheduled()) { // ackSent?
//        tcpEV << "ACK has already been sent (possibly piggybacked on data)\n";
    }
    else {
        if (!state->delayed_acks_enabled) { // delayed ACK disabled
            EV_INFO << "rcv_nxt changed to " << state->rcv_nxt << ", (delayed ACK disabled) sending ACK now\n";
            conn->sendAck();
        }
        else if (state->adaptiveDelayedAcks) {
            // Linux-shaped decision (__tcp_ack_snd_check): immediate ACK when
            // more than one full frame is pending, in quickack mode, or when
            // protocol state demands one; otherwise arm the ADAPTIVE delayed
            // ACK. The ATO bookkeeping runs first (tcp_event_data_recv).
            dataArrivedAtoUpdate();
            uint32_t mss = state->snd_mss > 0 ? state->snd_mss : 536;
            bool moreThanOneFrame = (state->rcv_nxt - state->last_ack_sent) > mss;
            // An arrival accepted BEYOND the advertised-window promise (the
            // empty-queue over-accept) is not immediate-ACKed by the kernel --
            // its selftest pins this ("It does not trigger an immediate ACK",
            // rcv_neg_window) -- so suppress the quickack/multi-frame immediate
            // arms for this decision; a protocol-mandated ack_now still wins.
            bool overAccept = conn->overWindowAcceptPending;
            conn->overWindowAcceptPending = false;
            if (state->ack_now || ((moreThanOneFrame || inQuickackMode()) && !overAccept)) {
                EV_INFO << "rcv_nxt changed to " << state->rcv_nxt << ", sending immediate ACK ("
                        << (state->ack_now ? "ack_now" : moreThanOneFrame ? "second full frame" : "quickack mode")
                        << ", quickack budget " << state->quickAckCounter << ")\n";
                conn->sendAck();
            }
            else {
                EV_INFO << "rcv_nxt changed to " << state->rcv_nxt << ", arming adaptive delayed ACK (ato="
                        << state->ackAto << ")\n";
                scheduleDelayedAck();
            }
        }
        else { // delayed ACK enabled
            if (state->ack_now) {
                EV_INFO << "rcv_nxt changed to " << state->rcv_nxt << ", (delayed ACK enabled, but ack_now is set) sending ACK now\n";
                conn->sendAck();
            }
            // RFC 1122, page 96: "in a stream of full-sized segments there SHOULD be an ACK for at least every second segment."
            else if (state->full_sized_segment_counter >= state->delayedAckFrameCount) {
                EV_INFO << "rcv_nxt changed to " << state->rcv_nxt << ", (delayed ACK enabled, but full_sized_segment_counter=" << state->full_sized_segment_counter << ") sending ACK now\n";
                conn->sendAck();
            }
            else {
                EV_INFO << "rcv_nxt changed to " << state->rcv_nxt << ", (delayed ACK enabled and full_sized_segment_counter=" << state->full_sized_segment_counter << ") scheduling ACK\n";
                if (!delayedAckTimer->isScheduled()) // schedule delayed ACK timer if not already running
                    conn->scheduleAfter(delayedAckTimeout, delayedAckTimer);
            }
        }
    }
}

void TcpAlgorithmBase::receivedAckForAlreadyAckedData(const TcpHeader *tcpHeader, uint32_t payloadLength)
{
    // A pure window-update ACK that reopened a closed window ends the persist
    // state and transmits queued data immediately (Linux FLAG_WIN_UPDATE ->
    // tcp_data_snd_check; without this the data waited for the next
    // zero-window probe's ACK, one whole doubled persist period late).
    // Not gated on the persist timer being armed: when the ZERO window came
    // with the handshake itself, nothing was ever in flight, no ACK ever
    // acked data, and the persist timer was never started -- yet queued data
    // must still go out the moment the window opens (tcp-info-rwnd-limited
    // pins it). Restricted to nothing-in-flight so ordinary dupacks during
    // loss recovery never reach the send path from here.
    if (state->snd_wnd > 0 && state->snd_una == state->snd_max) {
        if (persistTimer->isScheduled()) {
            EV_INFO << "Window reopened by a pure window update: canceling PERSIST timer\n";
            cancelEvent(persistTimer);
            state->persist_factor = 0;
        }
        sendData(false);
    }

    countDuplicateAck(tcpHeader, payloadLength);

    //
    // Leave congestion window management and possible sending data to
    // subclasses (e.g. TcpTahoe, TcpReno).
    //
    // That is, subclasses will redefine this method, call us, then perform
    // window adjustments and send data (if there's room in the window).
    //
}

bool TcpAlgorithmBase::isDuplicateAck(const TcpHeader *tcpHeader, uint32_t payloadLength)
{
    return state->snd_una == tcpHeader->getAckNo() && payloadLength == 0 && state->snd_una != state->snd_max;
}

void TcpAlgorithmBase::countDuplicateAck(const TcpHeader *tcpHeader, uint32_t payloadLength)
{
    if (isDuplicateAck(tcpHeader, payloadLength)) {
        // during loss recovery the recovery strategy owns the counter
        if (!state->lossRecovery) {
            state->dupacks++;
            conn->emit(dupAcksSignal, state->dupacks);
        }
        receivedDuplicateAck();
    }
    else {
        // if doesn't qualify as duplicate ACK, just ignore it.
        if (payloadLength == 0) {
            if (state->snd_una != tcpHeader->getAckNo())
                EV_DETAIL << "Old ACK: ackNo < snd_una\n";
            else if (state->snd_una == state->snd_max)
                EV_DETAIL << "ACK looks duplicate but we have currently no unacked data (snd_una == snd_max)\n";
        }
        // reset counter
        state->dupacks = 0;
        conn->emit(dupAcksSignal, state->dupacks);
    }
}

void TcpAlgorithmBase::receivedAckForUnackedData(uint32_t firstSeqAcked)
{
    if (!state->ts_enabled) {
        // if round-trip time measurement is running, check if rtseq has been acked
        if (state->rtseq_sendtime != 0 && seqLess(state->rtseq, state->snd_una)) {
            // print value
            EV_DETAIL << "Round-trip time measured on rtseq=" << state->rtseq << ": "
                      << floor((simTime() - state->rtseq_sendtime) * 1000 + 0.5) << "ms\n";

            rttMeasurementComplete(state->rtseq_sendtime, simTime()); // update RTT variables with new value

            // measurement finished
            state->rtseq_sendtime = 0;
        }
    }

    //
    // handling of retransmission timer: if the ACK is for the last segment sent
    // (no data in flight), cancel the timer, otherwise restart the timer
    // with the current RTO value.
    //
    if (state->snd_una == state->snd_max) {
        if (rexmitTimer->isScheduled()) {
            EV_INFO << "ACK acks all outstanding segments, cancel REXMIT timer\n";
            cancelEvent(rexmitTimer);
        }
        else
            EV_INFO << "There were no outstanding segments, nothing new in this ACK.\n";
    }
    else {
        EV_INFO << "ACK acks some but not all outstanding segments ("
                << (state->snd_max - state->snd_una) << " bytes outstanding), "
                << "restarting REXMIT timer\n";
        cancelEvent(rexmitTimer);
        startRexmitTimer();
    }

    //
    // handling of PERSIST timer:
    // If data sender received a zero-sized window, check retransmission timer.
    //  If retransmission timer is not scheduled, start PERSIST timer if not already
    //  running.
    //
    // If data sender received a non zero-sized window, check PERSIST timer.
    //  If PERSIST timer is scheduled, cancel PERSIST timer.
    //
    if (state->snd_wnd == 0) { // received zero-sized window?
        if (rexmitTimer->isScheduled()) {
            if (persistTimer->isScheduled()) {
                EV_INFO << "Received zero-sized window and REXMIT timer is running therefore PERSIST timer is canceled.\n";
                cancelEvent(persistTimer);
                state->persist_factor = 0;
            }
            else
                EV_INFO << "Received zero-sized window and REXMIT timer is running therefore PERSIST timer is not started.\n";
        }
        else {
            if (!persistTimer->isScheduled()) {
                EV_INFO << "Received zero-sized window therefore PERSIST timer is started.\n";
                // Linux probe0: the first probe fires one RTO after the
                // window closed; subsequent probes double from there
                state->persist_timeout = state->rexmit_timeout;
                conn->scheduleAfter(state->persist_timeout, persistTimer);
            }
            else
                EV_INFO << "Received zero-sized window and PERSIST timer is already running.\n";
        }
    }
    else { // received non zero-sized window?
        if (persistTimer->isScheduled()) {
            EV_INFO << "Received non zero-sized window therefore PERSIST timer is canceled.\n";
            cancelEvent(persistTimer);
            state->persist_factor = 0;
        }
    }

    state->dupacks = 0;
    conn->emit(dupAcksSignal, state->dupacks);

    //
    // Leave congestion window management and possible sending data to
    // subclasses (e.g. TcpTahoe, TcpReno).
    //
    // That is, subclasses will redefine this method, call us, then perform
    // window adjustments and send data (if there's room in the window).
    //
}

void TcpAlgorithmBase::receivedDuplicateAck()
{
    EV_INFO << "Duplicate ACK #" << state->dupacks << "\n";

    bool fullSegmentsOnly = state->nagle_enabled && state->snd_una != state->snd_max;
    if (state->dupacks < state->dupthresh && state->limited_transmit_enabled) // DUPTRESH = 3
        conn->sendOneNewSegment(fullSegmentsOnly, state->snd_cwnd); // RFC 3042

    //
    // Leave to subclasses (e.g. TcpTahoe, TcpReno) whatever they want to do
    // on duplicate Acks.
    //
    // That is, subclasses will redefine this method, call us, then perform
    // whatever action they want to do on dupAcks (e.g. retransmitting one segment).
    //
}

void TcpAlgorithmBase::receivedAckForUnsentData(uint32_t seq)
{
    // Note: In this case no immediate ACK will be send because not mentioned
    // in [Stevens, W.R.: TCP/IP Illustrated, Volume 2, page 861].
    // To force immediate ACK use:
//    state->ack_now = true;
//    tcpEV << "ACK acks something not yet sent, sending immediate ACK\n";
    EV_INFO << "ACK acks something not yet sent, sending ACK\n";
    conn->sendAck();
    state->dupacks = 0;
    conn->emit(dupAcksSignal, state->dupacks);
}

void TcpAlgorithmBase::ackSent()
{
    // every ACK actually sent consumes one unit of the quickack budget
    // (Linux tcp_event_ack_sent -> tcp_dec_quickack_mode)
    if (state->adaptiveDelayedAcks && state->quickAckCounter > 0)
        state->quickAckCounter--;
    state->full_sized_segment_counter = 0; // reset counter
    state->ack_now = false; // reset flag
    state->last_ack_sent = state->rcv_nxt; // update last_ack_sent, needed for TS option
    // if delayed ACK timer is running, cancel it
    if (delayedAckTimer->isScheduled())
        cancelEvent(delayedAckTimer);
}

void TcpAlgorithmBase::dataSent(uint32_t fromseq)
{
    // a data reply within one ATO of the last received packet is interactive
    // ("pingpong") evidence -- it makes the receiver favor delayed ACKs
    // (Linux tcp_event_data_sent, called for every data-bearing transmit;
    // lrcvtime is seeded at connection establishment, see established())
    if (state->adaptiveDelayedAcks && state->lastDataRecvTime > SIMTIME_ZERO
        && simTime() - state->lastDataRecvTime < state->ackAto
        && state->pingpongCount < TCP_PINGPONG_THRESH)
    {
        state->pingpongCount++;
    }

    // if retransmission timer not running, schedule it
    if (!rexmitTimer->isScheduled()) {
        EV_INFO << "Starting REXMIT timer\n";
        startRexmitTimer();
    }

    // RFC 8985 7.2: (re)arm the loss probe for the new tail of the flight
    schedulePto();

    if (!state->ts_enabled) {
        // start round-trip time measurement (if not already running)
        if (state->rtseq_sendtime == 0) {
            // remember this sequence number and when it was sent
            state->rtseq = fromseq;
            state->rtseq_sendtime = simTime();
            EV_DETAIL << "Starting rtt measurement on seq=" << state->rtseq << "\n";
        }
    }

    state->time_last_data_sent = simTime();

    // record per-segment transmit times (shared facility used by Vegas/Westwood
    // RTT sampling, and by RACK/Eifel loss recovery)
    state->sentInfo.clearTo(state->snd_una);
    // Loss probes and post-RTO retransmissions can move snd_nxt backwards, so a
    // send may start below the range this list currently covers (it only records
    // forward progress). Recording such a range would violate the list's
    // contiguity invariant; the segment's timing is already tracked per-region in
    // the rexmit queue, which is what RACK reads, so skip it here.
    if (seqLess(fromseq, state->snd_max) && state->sentInfo.isInRange(fromseq))
        state->sentInfo.set(fromseq, state->snd_max, simTime());
}

void TcpAlgorithmBase::segmentRetransmitted(uint32_t fromseq, uint32_t toseq)
{
    if (seqLess(fromseq, toseq) && state->sentInfo.isInRange(fromseq))
        state->sentInfo.set(fromseq, toseq, simTime());
}

void TcpAlgorithmBase::restartRexmitTimer()
{
    if (rexmitTimer->isScheduled())
        cancelEvent(rexmitTimer);

    startRexmitTimer();
}

bool TcpAlgorithmBase::shouldMarkAck()
{

    // RFC 3168, pages 19-20:
    // "When TCP receives a CE data packet at the destination end-system, the
    // TCP data receiver sets the ECN-Echo flag in the TCP header of the
    // subsequent ACK packet.
    // ...
    // After a TCP receiver sends an ACK packet with the ECN-Echo bit set,
    // that TCP receiver continues to set the ECN-Echo flag in all the ACK
    // packets it sends (whether they acknowledge CE data packets or non-CE
    // data packets) until it receives a CWR packet (a packet with the CWR
    // flag set).  After the receipt of the CWR packet, acknowledgments for
    // subsequent non-CE data packets do not have the ECN-Echo flag set."

    if (state && state->ect) {
        if (state->gotCeIndication) {
            EV_INFO << "Received CE... ";
            if (state->ecnEchoState)
                EV_INFO << "Already in ecnEcho state\n";
            else {
                state->ecnEchoState = true;
                EV << "Entering ecnEcho state\n";
            }
            state->gotCeIndication = false;
        }
        return state->ecnEchoState;
    }
    return false;
}

void TcpAlgorithmBase::processEcnInEstablished()
{
}

uint32_t TcpAlgorithmBase::getBytesInFlight() const
{
    return state->snd_nxt - conn->getDataSndUna();
}

uint32_t TcpAlgorithmBase::calculateSsthreshForFastRecovery()
{
    // Default (RFC 5681 / RFC 6675 4.2): ssthresh = max(FlightSize/2, 2*SMSS),
    // used by the Reno family; CUBIC overrides with cwnd*beta.
    return std::max(getBytesInFlight() / 2, 2 * state->snd_mss);
}

} // namespace tcp
} // namespace inet

