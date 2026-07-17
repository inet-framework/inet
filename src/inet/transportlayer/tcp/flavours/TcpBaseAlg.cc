//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009-2010 Thomas Reschka
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/TcpBaseAlg.h"

#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"

namespace inet {
namespace tcp {

//
// Some constants below. MIN_REXMIT_TIMEOUT is the minimum allowed retransmit
// interval.  It is currently one second but e.g. a FreeBSD kernel comment says
// it "will ultimately be reduced to 3 ticks for algorithmic stability,
// leaving the 200ms variance to deal with delayed-acks, protocol overheads.
// A 1 second minimum badly breaks throughput on any network faster then
// a modem that has minor but continuous packet loss unrelated to congestion,
// such as on a wireless network."
//
// RFC 1122, page 95:
// "A TCP SHOULD implement a delayed ACK, but an ACK should not
// be excessively delayed; in particular, the delay MUST be
// less than 0.5 seconds, and in a stream of full-sized
// segments there SHOULD be an ACK for at least every second
// segment."

#define DELAYED_ACK_TIMEOUT    0.2   // 200ms (RFC 1122: MUST be less than 0.5 seconds)
#define MAX_REXMIT_COUNT       12   // 12 retries
// RTO bounds are configurable via the minRto/maxRto parameters (state->min_rto,
// state->max_rto); the defaults reproduce the historical 1s / 2*MSL (RFC 1122) clamps.
#define MIN_PERSIST_TIMEOUT    5   // 5s
#define MAX_PERSIST_TIMEOUT    60   // 60s

std::string TcpBaseAlgStateVariables::str() const
{
    std::stringstream out;
    out << TcpStateVariables::str();
    out << " snd_cwnd=" << snd_cwnd;
    out << " rto=" << rexmit_timeout;
    return out.str();
}

std::string TcpBaseAlgStateVariables::detailedInfo() const
{
    std::stringstream out;
    out << TcpStateVariables::detailedInfo();
    out << "snd_cwnd=" << snd_cwnd << "\n";
    out << "rto=" << rexmit_timeout << "\n";
    out << "persist_timeout=" << persist_timeout << "\n";
    // TODO add others too
    return out.str();
}

simsignal_t TcpBaseAlg::cwndSignal = cComponent::registerSignal("cwnd"); // will record changes to snd_cwnd
simsignal_t TcpBaseAlg::ssthreshSignal = cComponent::registerSignal("ssthresh"); // will record changes to ssthresh
simsignal_t TcpBaseAlg::rttSignal = cComponent::registerSignal("rtt"); // will record measured RTT
simsignal_t TcpBaseAlg::srttSignal = cComponent::registerSignal("srtt"); // will record smoothed RTT
simsignal_t TcpBaseAlg::rttvarSignal = cComponent::registerSignal("rttvar"); // will record RTT variance (rttvar)
simsignal_t TcpBaseAlg::rtoSignal = cComponent::registerSignal("rto"); // will record retransmission timeout
simsignal_t TcpBaseAlg::numRtosSignal = cComponent::registerSignal("numRtos"); // will record total number of RTOs

TcpBaseAlg::TcpBaseAlg() : TcpAlgorithm(),
    state((TcpBaseAlgStateVariables *&)TcpAlgorithm::state)
{
    rexmitTimer = persistTimer = delayedAckTimer = keepAliveTimer = tlpTimer = nullptr;
}

TcpBaseAlg::~TcpBaseAlg()
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
}

void TcpBaseAlg::initialize()
{
    TcpAlgorithm::initialize();

    rexmitTimer = new cMessage("REXMIT");
    persistTimer = new cMessage("PERSIST");
    delayedAckTimer = new cMessage("DELAYEDACK");
    keepAliveTimer = new cMessage("KEEPALIVE");
    tlpTimer = new cMessage("TLP-PTO");

    rexmitTimer->setContextPointer(conn);
    persistTimer->setContextPointer(conn);
    delayedAckTimer->setContextPointer(conn);
    keepAliveTimer->setContextPointer(conn);
    tlpTimer->setContextPointer(conn);

    state->keepalive_enabled = conn->getTcpMain()->par("keepAliveEnabled");
    state->keepalive_idle_time = conn->getTcpMain()->par("keepAliveIdleTime");
    state->keepalive_interval = conn->getTcpMain()->par("keepAliveInterval");
    state->keepalive_max_probes = conn->getTcpMain()->par("keepAliveProbeCount");

    state->rexmit_timeout = conn->getTcpMain()->par("initialRto");
    state->min_rto = conn->getTcpMain()->par("minRto");
    state->max_rto = conn->getTcpMain()->par("maxRto");
}

uint32_t TcpBaseAlg::initialWindow() const
{
    switch (state->init_cwnd_mode) {
        case 1: // RFC 3390
            return std::min(4 * state->snd_mss, std::max(2 * state->snd_mss, (uint32_t)4380));
        case 2: // RFC 6928 (IW10)
            return std::min(10 * state->snd_mss, std::max(2 * state->snd_mss, (uint32_t)14600));
        default: // RFC 2001: one segment
            return state->snd_mss;
    }
}

void TcpBaseAlg::established(bool active)
{
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
    // RFC 2581 [RFC2581], which specified that the congestion window be
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
    // RFC 2001, page 3:
    // " 1.  Initialization for a given connection sets cwnd to one segment
    // and ssthresh to 65535 bytes."
    else
        state->snd_cwnd = state->snd_mss;

    if (active) {
        // finish connection setup with ACK (possibly piggybacked on data)
        EV_INFO << "Completing connection setup by sending ACK (possibly piggybacked on data)\n";
        if (!sendData(false)) // FIXME - This condition is never true because the buffer is empty (at this time) therefore the first ACK is never piggyback on data
            conn->sendAck();
    }

    if (state->keepalive_enabled) {
        state->time_last_segment_received = simTime();
        state->keepalive_probes_sent = 0;
        conn->scheduleAfter(state->keepalive_idle_time, keepAliveTimer);
    }
}

void TcpBaseAlg::connectionClosed()
{
    cancelEvent(rexmitTimer);
    cancelEvent(persistTimer);
    cancelEvent(delayedAckTimer);
    cancelEvent(keepAliveTimer);
}

void TcpBaseAlg::processTimer(cMessage *timer, TcpEventCode& event)
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
    else
        throw cRuntimeError(timer, "unrecognized timer");
}

void TcpBaseAlg::processRexmitTimer(TcpEventCode& event)
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
    // in RFC 793 above, we'll leave it to subclasses (e.g. TcpTahoe, TcpReno).
    //
    if (++state->rexmit_count > MAX_REXMIT_COUNT) {
        EV_DETAIL << "Retransmission count exceeds " << MAX_REXMIT_COUNT << ", aborting connection\n";
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
    if (state->rexmit_timeout > state->max_rto)
        state->rexmit_timeout = state->max_rto;

    conn->scheduleAfter(state->rexmit_timeout, rexmitTimer);

    EV_INFO << " to " << state->rexmit_timeout << "s, and cancelling RTT measurement\n";

    // cancel round-trip time measurement
    state->rtseq_sendtime = 0;

    state->numRtos++;

    conn->emit(numRtosSignal, state->numRtos);

    // if sacked_enabled reset sack related flags
    if (state->sack_enabled) {
        conn->getRexmitQueueForUpdate()->resetSackedBit();
        conn->getRexmitQueueForUpdate()->resetRexmittedBit();

        // RFC 3517, page 8: "If an RTO occurs during loss recovery as specified in this document,
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

void TcpBaseAlg::processPersistTimer(TcpEventCode& event)
{
    // setup and restart the PERSIST timer
    // FIXME Calculation of PERSIST timer is not as simple as done here!
    // It depends on RTT calculations and is bounded to 5-60 seconds.
    // This simplified PERSIST timer calculation generates values
    // as presented in [Stevens, W.R.: TCP/IP Illustrated, Volume 1, chapter 22.2]
    // (5, 5, 6, 12, 24, 48, 60, 60, 60...)
    if (state->persist_factor == 0)
        state->persist_factor++;
    else if (state->persist_factor < 64)
        state->persist_factor = state->persist_factor * 2;

    state->persist_timeout = state->persist_factor * 1.5; // 1.5 is a factor for typical LAN connection [Stevens, W.R.: TCP/IP Ill. Vol. 1, chapter 22.2]

    // PERSIST timer is bounded to 5-60 seconds
    if (state->persist_timeout < MIN_PERSIST_TIMEOUT)
        state->persist_timeout = MIN_PERSIST_TIMEOUT;

    if (state->persist_timeout > MAX_PERSIST_TIMEOUT)
        state->persist_timeout = MAX_PERSIST_TIMEOUT;

    conn->scheduleAfter(state->persist_timeout, persistTimer);

    // sending persist probe
    conn->sendProbe();
    state->zeroWindowProbesSent++;
}

void TcpBaseAlg::processDelayedAckTimer(TcpEventCode& event)
{
    state->ack_now = true;
    conn->sendAck();
}

void TcpBaseAlg::processKeepAliveTimer(TcpEventCode& event)
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

void TcpBaseAlg::startRexmitTimer()
{
    // single-slot discipline with the loss-probe timer (Linux shares one
    // icsk timer slot between RETRANS and LOSS_PROBE): arming the RTO always
    // disarms a pending probe.
    if (tlpTimer != nullptr && tlpTimer->isScheduled())
        conn->cancelEvent(tlpTimer);

    // start counting retransmissions for this seq number.
    // Note: state->rexmit_timeout is set from rttMeasurementComplete().
    state->rexmit_count = 0;

    // schedule timer
    conn->scheduleAfter(state->rexmit_timeout, rexmitTimer);
}

void TcpBaseAlg::schedulePto()
{
    // Linux tcp_schedule_loss_probe: eligible while SACK-capable, not in loss
    // recovery, with no SACKed data outstanding (ca_state Open/CWR) and no
    // probe already in flight. The PTO replaces the RTO in the timer slot.
    if (!state->tlpEnabled || !state->sack_enabled || state->lossRecovery
            || state->sackedBytes != 0 || state->tlpHighSeq != 0
            || state->snd_una == state->snd_max)
        return;

    // PTO = 2*SRTT; with a single packet in flight add the peer's potential
    // delayed-ACK wait (Linux adds tcp_rto_min there). No RTT sample yet ->
    // the initial RTO (Linux TCP_TIMEOUT_INIT).
    simtime_t pto;
    if (state->srtt > 0) {
        pto = state->srtt * 2;
        if (state->snd_max - state->snd_una <= state->snd_mss)
            pto += state->min_rto; // single packet in flight: allow for the peer's delayed ACK
        else
            pto += SimTime(2, SIMTIME_MS); // Linux TCP_TIMEOUT_MIN_US: floors the PTO so a
                                           // near-zero srtt (e.g. a same-instant packetdrill
                                           // handshake) cannot fire the probe between
                                           // back-to-back ACKs of the same flight
    }
    else
        pto = state->rexmit_timeout;

    // never fire later than the RTO would have
    if (rexmitTimer->isScheduled()) {
        simtime_t rtoRemaining = rexmitTimer->getArrivalTime() - simTime();
        if (rtoRemaining < pto)
            pto = rtoRemaining;
        conn->cancelEvent(rexmitTimer);
    }
    else if (pto > state->rexmit_timeout)
        pto = state->rexmit_timeout;

    conn->rescheduleAfter(pto, tlpTimer);
    EV_DETAIL << "TLP: probe timeout armed in " << pto << "s (replaces RTO)\n";
}

void TcpBaseAlg::processPtoTimer(TcpEventCode& event)
{
    // Linux tcp_send_loss_probe. At most one probe per flight; if recovery
    // began (or everything got acked) since the PTO was armed, just fall back
    // to the ordinary RTO discipline.
    if (state->tlpHighSeq == 0 && !state->lossRecovery && state->snd_una != state->snd_max) {
        if (conn->sendTlpProbe()) {
            state->tlpHighSeq = state->snd_max;
            EV_INFO << "TLP: probe sent, tlpHighSeq=" << state->tlpHighSeq << "\n";
        }
    }
    startRexmitTimer(); // full RTO from now (Linux tcp_rearm_rto after the probe)
}

void TcpBaseAlg::rttMeasurementComplete(simtime_t tSent, simtime_t tAcked)
{
    //
    // Jacobson's algorithm for estimating RTT and adaptively setting RTO.
    //
    // Note: this implementation calculates in doubles. An impl. which uses
    // 500ms ticks is available from old tcpmodule.cc:calcRetransTimer().
    //

    // RTT estimator per RFC 6298 (Jacobson/Karn), with Linux's variance-floor RTO.
    simtime_t newRTT = tAcked - tSent;

    // track the minimum RTT (RACK loss detection); a running min (not windowed)
    if (newRTT > 0 && (state->minRtt == 0 || newRTT < state->minRtt))
        state->minRtt = newRTT;

    simtime_t& srtt = state->srtt;
    simtime_t& rttvar = state->rttvar;

    if (!state->rtt_measured) {
        // RFC 6298 (2.2): first measurement R -> SRTT = R, RTTVAR = R/2.
        // (Linux seeds mdev=2R -> rttvar=R here; we take the RFC's R/2. Documented divergence.)
        srtt = newRTT;
        rttvar = newRTT / 2;
        state->rtt_measured = true;
    }
    else {
        // RFC 6298 (2.3): RTTVAR uses the OLD SRTT, then SRTT is updated.
        //   RTTVAR = (1 - beta) * RTTVAR + beta * |SRTT - R|,   beta = 1/4
        //   SRTT   = (1 - alpha) * SRTT + alpha * R,            alpha = 1/8
        rttvar = 0.75 * rttvar + 0.25 * fabs(srtt - newRTT);
        srtt = 0.875 * srtt + 0.125 * newRTT;
    }

    // Linux-style variance floor (tcp_set_rto): RTO = SRTT + max(4*RTTVAR, RTO_MIN),
    // giving RTO >= SRTT + min_rto. This matches packetdrill-style comparisons; no
    // separate lower clamp needed.
    simtime_t varTerm = 4 * rttvar;
    if (varTerm < state->min_rto)
        varTerm = state->min_rto;
    simtime_t rto = srtt + varTerm;
    if (rto > state->max_rto)
        rto = state->max_rto;

    state->rexmit_timeout = rto;

    // record statistics
    EV_DETAIL << "Measured RTT=" << (newRTT * 1000) << "ms, updated SRTT=" << (srtt * 1000)
              << "ms, new RTO=" << (rto * 1000) << "ms\n";

    conn->emit(rttSignal, newRTT);
    conn->emit(srttSignal, srtt);
    conn->emit(rttvarSignal, rttvar);
    conn->emit(rtoSignal, rto);
}

void TcpBaseAlg::rttMeasurementCompleteUsingTS(uint32_t echoedTS)
{
    ASSERT(state->ts_enabled);

    // Note: The TS option is using uint32_t values (ms precision) therefore we convert the current simTime also to a uint32_t value (ms precision)
    // and then convert back to simtime_t to use rttMeasurementComplete() to update srtt and rttvar
    uint32_t now = conn->convertSimtimeToTS(simTime());
    simtime_t tSent = conn->convertTSToSimtime(echoedTS);
    simtime_t tAcked = conn->convertTSToSimtime(now);
    rttMeasurementComplete(tSent, tAcked);
}

bool TcpBaseAlg::sendData(bool sendCommandInvoked)
{
    // RFC 2581, pages 7 and 8: "When TCP has not received a segment for
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

void TcpBaseAlg::sendCommandInvoked()
{
    // try sending
    sendData(true);
}

void TcpBaseAlg::receivedOutOfOrderSegment()
{
    state->ack_now = true;
    EV_INFO << "Out-of-order segment, sending immediate ACK\n";
    conn->sendAck();
}

void TcpBaseAlg::receiveSeqChanged()
{
    // If we send a data segment already (with the updated seqNo) there is no need to send an additional ACK
    if (state->full_sized_segment_counter == 0 && !state->ack_now && state->last_ack_sent == state->rcv_nxt && !delayedAckTimer->isScheduled()) { // ackSent?
//        tcpEV << "ACK has already been sent (possibly piggybacked on data)\n";
    }
    else {
        // RFC 2581, page 6:
        // "3.2 Fast Retransmit/Fast Recovery
        // (...)
        // In addition, a TCP receiver SHOULD send an immediate ACK
        // when the incoming segment fills in all or part of a gap in the
        // sequence space."
        if (state->lossRecovery)
            state->ack_now = true; // although not mentioned in [Stevens, W.R.: TCP/IP Illustrated, Volume 2, page 861] seems like we have to set ack_now

        if (!state->delayed_acks_enabled) { // delayed ACK disabled
            EV_INFO << "rcv_nxt changed to " << state->rcv_nxt << ", (delayed ACK disabled) sending ACK now\n";
            conn->sendAck();
        }
        else { // delayed ACK enabled
            if (state->ack_now) {
                EV_INFO << "rcv_nxt changed to " << state->rcv_nxt << ", (delayed ACK enabled, but ack_now is set) sending ACK now\n";
                conn->sendAck();
            }
            // RFC 1122, page 96: "in a stream of full-sized segments there SHOULD be an ACK for at least every second segment."
            else if (state->full_sized_segment_counter >= 2) {
                EV_INFO << "rcv_nxt changed to " << state->rcv_nxt << ", (delayed ACK enabled, but full_sized_segment_counter=" << state->full_sized_segment_counter << ") sending ACK now\n";
                conn->sendAck();
            }
            else {
                EV_INFO << "rcv_nxt changed to " << state->rcv_nxt << ", (delayed ACK enabled and full_sized_segment_counter=" << state->full_sized_segment_counter << ") scheduling ACK\n";
                if (!delayedAckTimer->isScheduled()) // schedule delayed ACK timer if not already running
                    conn->scheduleAfter(DELAYED_ACK_TIMEOUT, delayedAckTimer);
            }
        }
    }
}

void TcpBaseAlg::receivedDataAck(uint32_t firstSeqAcked)
{
    // Tail Loss Probe outcome (Linux tcp_process_tlp_ack): the ACK reached the
    // probe's snd_max. A new-data probe acked, or a D-SACK on this ACK (both
    // the original and the probe arrived), ends the episode benignly. A
    // retransmitted probe acked WITHOUT a D-SACK means the original tail was
    // really lost and the probe silently repaired it -- apply the CWR-style
    // congestion response (Linux: tcp_init_cwnd_reduction + end, cwnd=ssthresh).
    if (state->tlpHighSeq != 0 && seqGE(state->snd_una, state->tlpHighSeq)) {
        if (state->tlpRetrans && !state->dsackSeen) {
            EV_INFO << "TLP: probe repaired a real tail loss, applying congestion response\n";
            tlpLossEpisode();
        }
        state->tlpHighSeq = 0;
    }

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
        if (tlpTimer != nullptr && tlpTimer->isScheduled())
            cancelEvent(tlpTimer); // nothing left to probe
    }
    else {
        EV_INFO << "ACK acks some but not all outstanding segments ("
                << (state->snd_max - state->snd_una) << " bytes outstanding), "
                << "restarting REXMIT timer\n";
        cancelEvent(rexmitTimer);
        startRexmitTimer();
        schedulePto(); // data still in flight: re-arm the loss probe when eligible
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

    //
    // Leave congestion window management and possible sending data to
    // subclasses (e.g. TcpTahoe, TcpReno).
    //
    // That is, subclasses will redefine this method, call us, then perform
    // window adjustments and send data (if there's room in the window).
    //
}

void TcpBaseAlg::receivedDuplicateAck()
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

void TcpBaseAlg::receivedAckForDataNotYetSent(uint32_t seq)
{
    // Note: In this case no immediate ACK will be send because not mentioned
    // in [Stevens, W.R.: TCP/IP Illustrated, Volume 2, page 861].
    // To force immediate ACK use:
//    state->ack_now = true;
//    tcpEV << "ACK acks something not yet sent, sending immediate ACK\n";
    EV_INFO << "ACK acks something not yet sent, sending ACK\n";
    conn->sendAck();
}

void TcpBaseAlg::ackSent()
{
    state->full_sized_segment_counter = 0; // reset counter
    state->ack_now = false; // reset flag
    state->last_ack_sent = state->rcv_nxt; // update last_ack_sent, needed for TS option
    // if delayed ACK timer is running, cancel it
    if (delayedAckTimer->isScheduled())
        cancelEvent(delayedAckTimer);
}

void TcpBaseAlg::dataSent(uint32_t fromseq)
{
    // if retransmission timer not running, schedule it
    if (!rexmitTimer->isScheduled() && !(tlpTimer != nullptr && tlpTimer->isScheduled())) {
        EV_INFO << "Starting REXMIT timer\n";
        startRexmitTimer();
    }

    // possibly convert the armed RTO into a tail-loss-probe timeout
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
    state->sentInfo.set(fromseq, state->snd_max, simTime());

    // RFC 6937 PRR: count bytes transmitted while in fast recovery
    if (state->prrEnabled && state->lossRecovery && seqGreater(state->snd_max, fromseq))
        state->prrOut += state->snd_max - fromseq;
}

void TcpBaseAlg::segmentRetransmitted(uint32_t fromseq, uint32_t toseq)
{
    state->sentInfo.set(fromseq, toseq, simTime());

    // Eifel (RFC 3522 / Linux retrans_stamp): stamp the FIRST retransmission of
    // the episode with our TS clock. An ACK later echoing a TSecr OLDER than
    // this was generated by the ORIGINAL transmission, proving it spurious.
    if (state->ts_enabled && state->retransStampTS == 0)
        state->retransStampTS = TcpConnection::convertSimtimeToTS(simTime());

    // RFC 6937 PRR: count retransmitted bytes while in fast recovery
    if (state->prrEnabled && state->lossRecovery && seqGreater(toseq, fromseq))
        state->prrOut += toseq - fromseq;

    // RFC 2883 loss undo: count the retransmitted segments of the current episode
    // so a later D-SACK of all of them can trigger an undo.
    if (state->lossUndoEnabled && state->undoMarker != 0 && seqGreater(toseq, fromseq)) {
        uint32_t segs = (toseq - fromseq + state->snd_mss - 1) / state->snd_mss;
        if (state->undoRetrans < 0)
            state->undoRetrans = (int32_t)segs;
        else
            state->undoRetrans += (int32_t)segs;
    }
}

void TcpBaseAlg::restartRexmitTimer()
{
    if (rexmitTimer->isScheduled())
        cancelEvent(rexmitTimer);

    startRexmitTimer();
}

bool TcpBaseAlg::shouldMarkAck()
{
    // rfc-3168, pages 19-20:
    // When TCP receives a CE data packet at the destination end-system, the
    // TCP data receiver sets the ECN-Echo flag in the TCP header of the
    // subsequent ACK packet.
    // ...
    // After a TCP receiver sends an ACK packet with the ECN-Echo bit set,
    // that TCP receiver continues to set the ECN-Echo flag in all the ACK
    // packets it sends (whether they acknowledge CE data packets or non-CE
    // data packets) until it receives a CWR packet (a packet with the CWR
    // flag set).  After the receipt of the CWR packet, acknowledgments for
    // subsequent non-CE data packets do not have the ECN-Echo flag set.

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

void TcpBaseAlg::processEcnInEstablished()
{
}

} // namespace tcp
} // namespace inet

