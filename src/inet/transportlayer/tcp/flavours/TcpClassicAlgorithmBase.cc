//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/tcp/flavours/TcpClassicAlgorithmBase.h"

#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"

namespace inet {
namespace tcp {

void TcpClassicAlgorithmBaseStateVariables::setSendQueueLimit(uint32_t newLimit)
{
    // The initial value of ssthresh SHOULD be set arbitrarily high (e.g.,
    // to the size of the largest possible advertised window) -> defined by sendQueueLimit
    sendQueueLimit = newLimit;
    ssthresh = sendQueueLimit;
}

std::string TcpClassicAlgorithmBaseStateVariables::str() const
{
    std::stringstream out;
    out << TcpAlgorithmBaseStateVariables::str();
    out << " ssthresh=" << ssthresh;
    return out.str();
}

std::string TcpClassicAlgorithmBaseStateVariables::detailedInfo() const
{
    std::stringstream out;
    out << TcpAlgorithmBaseStateVariables::detailedInfo();
    out << "ssthresh=" << ssthresh << "\n";
    return out.str();
}

// ---

TcpClassicAlgorithmBase::TcpClassicAlgorithmBase() : TcpAlgorithmBase(),
    state((TcpClassicAlgorithmBaseStateVariables *&)TcpAlgorithm::state)
{
}

void TcpClassicAlgorithmBase::initializeAlgorithm()
{
    TcpAlgorithmBase::initializeAlgorithm();
    state->ssthresh = conn->getTcpMain()->par("initialSsthresh");
    const char *slowStartExit = par("slowStartExit");
    state->slowStartExit = !strcmp(slowStartExit, "hystart") ? SLOW_START_EXIT_HYSTART
            : !strcmp(slowStartExit, "hystartpp") ? SLOW_START_EXIT_HYSTARTPP : SLOW_START_EXIT_NONE;
}

TcpClassicAlgorithmBase::~TcpClassicAlgorithmBase()
{
    delete congestionControl;
    delete recovery;
}

void TcpClassicAlgorithmBase::established(bool active)
{
    TcpAlgorithmBase::established(active);

    recovery = createRecovery();
    congestionControl = createCongestionControl();

    // RFC 9406 section 4.2: "Define windowEnd as a sequence number initialized to SND.NXT"
    state->hystartppSsthresh = state->ssthresh;
    state->hystartppWindowEnd = state->snd_nxt;
}

void TcpClassicAlgorithmBase::processRexmitTimer(TcpEventCode& event)
{
    TcpAlgorithmBase::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;

    if (recovery != nullptr)
        recovery->onRexmitTimeout();

    // a timeout ends the fast recovery
    state->lossRecovery = false;

    // RFC 5681, page 8:
    // "Furthermore, upon a timeout cwnd MUST be set to no more than the loss
    // window, LW, which equals 1 full-sized segment (regardless of the
    // value of IW).  Therefore, after retransmitting the dropped segment
    // the TCP sender uses the slow start algorithm to increase the window
    // from 1 full-sized segment to the new value of ssthresh, at which
    // point congestion avoidance again takes over."
    //
    // RFC 5681, page 7:
    // "When a TCP sender detects segment loss using the retransmission
    // timer and the given segment has not yet been resent by way of the
    // retransmission timer, the value of ssthresh MUST be set to no more
    // than the value given in equation (4):
    //
    //   ssthresh = max (FlightSize / 2, 2*SMSS)            (4)
    //
    // where, as discussed above, FlightSize is the amount of outstanding
    // data in the network."
    state->ssthresh = calculateSsthreshForRto();
    conn->emit(ssthreshSignal, state->ssthresh);

    state->snd_cwnd = calculateCwndForRto();
    conn->emit(cwndSignal, state->snd_cwnd);

    EV_INFO << "Begin Slow Start: resetting cwnd to " << state->snd_cwnd
            << ", ssthresh=" << state->ssthresh << "\n";

    state->afterRto = true;

    conn->markOutstandingLostOnRto();
    conn->retransmitOneSegment(true);
}

void TcpClassicAlgorithmBase::tlpLossResponse()
{
    // RFC 8985 section 7.4.2: "invoke a congestion control response equivalent to a
    // fast recovery". Linux runs tcp_init_cwnd_reduction() and
    // tcp_end_cwnd_reduction() at once: the ssthresh of the flavour, and cwnd = ssthresh.
    state->ssthresh = calculateSsthreshForFastRecovery();
    state->snd_cwnd = state->ssthresh;
    conn->emit(ssthreshSignal, state->ssthresh);
    conn->emit(cwndSignal, state->snd_cwnd);
    EV_INFO << "TLP: the probe repaired a lost tail, cwnd reduced to ssthresh=" << state->ssthresh << "\n";
}

void TcpClassicAlgorithmBase::receivedAckForUnackedData(uint32_t firstSeqAcked)
{
    TcpAlgorithmBase::receivedAckForUnackedData(firstSeqAcked);

    uint32_t numBytesAcked = state->snd_una - firstSeqAcked;
    bool inFastRecovery = isInFastRecovery();
    if (inFastRecovery) {
        recovery->receivedAckForUnackedData(numBytesAcked);
        ackedInFastRecovery(numBytesAcked);
    }
    else if (!processEce(numBytesAcked)) {
        if (!isHystartppActive() || !hystartppAck(firstSeqAcked, numBytesAcked))
            congestionControl->receivedAckForUnackedData(numBytesAcked);
    }

    ackProcessed(inFastRecovery);

    // Outside a fast recovery, an ACK of new data ends the guesses that the
    // duplicate ACKs before it made (Linux tcp_reset_reno_sack()).
    if (!state->sack_enabled && !state->lossRecovery)
        conn->getRexmitQueueForUpdate()->resetSackedBit();

    sendData(false);
    ensureRexmitTimerArmed();
}

void TcpClassicAlgorithmBase::dataSent(uint32_t fromseq)
{
    TcpAlgorithmBase::dataSent(fromseq);
    if (recovery != nullptr)
        recovery->dataSent(fromseq);
}

void TcpClassicAlgorithmBase::segmentRetransmitted(uint32_t fromseq, uint32_t toseq)
{
    TcpAlgorithmBase::segmentRetransmitted(fromseq, toseq);
    if (recovery != nullptr)
        recovery->segmentRetransmitted(fromseq, toseq);
}

void TcpClassicAlgorithmBase::segmentsAcked(uint32_t fromSeq, uint32_t toSeq)
{
    if (recovery != nullptr)
        recovery->segmentsAcked(fromSeq, toSeq);
}

bool TcpClassicAlgorithmBase::processEce(uint32_t numBytesAcked)
{
    if (state->ect && state->gotEce) {
        // RFC 3168, page 18
        // "If the sender receives an ECN-Echo (ECE) ACK
        // packet (that is, an ACK packet with the ECN-Echo flag set in the TCP
        // header), then the sender knows that congestion was encountered in the
        // network on the path from the sender to the receiver.  The indication
        // of congestion should be treated just as a congestion loss in non-
        // ECN-Capable TCP. That is, the TCP source halves the congestion window
        // "cwnd" and reduces the slow start threshold "ssthresh".  The sending
        // TCP SHOULD NOT increase the congestion window in response to the
        // receipt of an ECN-Echo ACK packet.
        // ...
        //   The value of the congestion window is bounded below by a value of one MSS.
        // ...
        //   TCP should not react to congestion indications more than once every
        // window of data (or more loosely, more than once every round-trip
        // time). That is, the TCP sender's congestion window should be reduced
        // only once in response to a series of dropped and/or CE packets from a
        // single window of data.  In addition, the TCP source should not decrease
        // the slow-start threshold, ssthresh, if it has been decreased
        // within the last round trip time."
        if (simTime() - state->eceReactionTime > state->srtt) {
            state->snd_cwnd = std::max(state->snd_cwnd / 2, state->snd_mss);
            conn->emit(cwndSignal, state->snd_cwnd);
            EV_INFO << "cwnd = cwnd / 2: received ECN-Echo ACK... new cwnd = " << state->snd_cwnd << "\n";

            state->ssthresh = state->snd_cwnd;
            conn->emit(ssthreshSignal, state->ssthresh);
            EV_INFO << "ssthresh = cwnd: received ECN-Echo ACK... new ssthresh = " << state->ssthresh << "\n";

            state->sndCwr = true;

            // RFC 3168, page 18
            // "The sending TCP MUST reset the retransmit timer on receiving
            // the ECN-Echo packet when the congestion window is one."
            if (state->snd_cwnd == state->snd_mss) {
                restartRexmitTimer();
                EV_INFO << "cwnd = 1 MSS... reset retransmit timer.\n";
            }
            state->eceReactionTime = simTime();
            state->gotEce = false;
            return true;
        }
        else
            EV_INFO << "multiple ECN-Echo ACKs in less than rtt... no ECN reaction\n";
        state->gotEce = false;
    }
    return false;
}

bool TcpClassicAlgorithmBase::isDuplicateAck(const TcpHeader *tcpHeader, uint32_t payloadLength)
{
    return recovery->isDuplicateAck(tcpHeader, payloadLength);
}

void TcpClassicAlgorithmBase::receivedAckForAlreadyAckedData(const TcpHeader *tcpHeader, uint32_t payloadLength)
{
    TcpAlgorithmBase::receivedAckForAlreadyAckedData(tcpHeader, payloadLength);

    // Outside a fast recovery, an old ACK that is no duplicate (for example data
    // of the peer) resets the duplicate-ACK counter. The inferred SACKs of the
    // duplicate ACKs before it go with it, so that before the fast retransmit
    // they give the room of at most two segments, as Limited Transmit does
    // (RFC 3042).
    if (!state->sack_enabled && !state->lossRecovery && state->dupacks == 0)
        conn->getRexmitQueueForUpdate()->resetSackedBit();
}

void TcpClassicAlgorithmBase::receivedDuplicateAck()
{
    // Without SACK, TcpAlgorithmBase sends the Limited Transmit data; with SACK,
    // the recovery sends it itself, by NextSeg()
    if (!state->sack_enabled) {
        // the duplicate ACK tells that one more segment has left the network
        // (Linux tcp_add_reno_sack())
        conn->getRexmitQueueForUpdate()->addInferredSack();
        TcpAlgorithmBase::receivedDuplicateAck();
    }

    recovery->receivedDuplicateAck();
}

simtime_t TcpClassicAlgorithmBase::getAckRttSample(uint32_t firstSeqAcked) const
{
    const TcpSegmentTransmitInfoList::Item *sent = state->sentInfo.get(firstSeqAcked);
    if (sent == nullptr || sent->getTransmitCount() != 1)
        return -1;
    return simTime() - sent->getFirstSentTime();
}

// the constants of RFC 9406 section 4.3; L = 8, because INET does not pace
static const double HYSTARTPP_MIN_RTT_THRESH = 0.004; // s
static const double HYSTARTPP_MAX_RTT_THRESH = 0.016; // s
static const int HYSTARTPP_MIN_RTT_DIVISOR = 8;
static const uint32_t HYSTARTPP_N_RTT_SAMPLE = 8;
static const uint32_t HYSTARTPP_CSS_GROWTH_DIVISOR = 4;
static const uint32_t HYSTARTPP_CSS_ROUNDS = 5;
static const uint32_t HYSTARTPP_L = 8;

bool TcpClassicAlgorithmBase::isHystartppActive()
{
    if (state->slowStartExit != SLOW_START_EXIT_HYSTARTPP || state->hystartppPhase == HYSTARTPP_ENDED)
        return false;
    // section 4.3: "An implementation SHOULD use HyStart++ only for the initial slow
    // start (when the ssthresh is at its initial value of arbitrarily high per
    // [RFC5681])": a congestion response, which changes ssthresh, ends it, and so does
    // the end of the slow start at ssthresh
    if (state->ssthresh != state->hystartppSsthresh || state->snd_cwnd >= state->ssthresh) {
        state->hystartppPhase = HYSTARTPP_ENDED;
        EV_INFO << "HyStart++: the first slow start is over\n";
        return false;
    }
    return true;
}

bool TcpClassicAlgorithmBase::hystartppAck(uint32_t firstSeqAcked, uint32_t numBytesAcked)
{
    // "When windowEnd is ACKed, the current round ends and windowEnd is set to SND.NXT."
    if (seqGE(state->snd_una, state->hystartppWindowEnd)) {
        state->hystartppLastRoundMinRtt = state->hystartppCurrentRoundMinRtt;
        state->hystartppCurrentRoundMinRtt = -1;
        state->hystartppRttSampleCount = 0;
        state->hystartppWindowEnd = state->snd_nxt;
        // "CSS lasts at most CSS_ROUNDS rounds. If the transition into CSS happens in
        // the middle of a round, that partial round counts towards the limit."
        if (state->hystartppPhase == HYSTARTPP_CSS && ++state->hystartppCssRounds >= HYSTARTPP_CSS_ROUNDS) {
            // "If CSS_ROUNDS rounds are complete, enter congestion avoidance by
            // setting the ssthresh to the current cwnd."
            state->ssthresh = state->snd_cwnd;
            conn->emit(ssthreshSignal, state->ssthresh);
            state->hystartppPhase = HYSTARTPP_ENDED;
            EV_INFO << "HyStart++: " << HYSTARTPP_CSS_ROUNDS << " rounds of CSS, congestion avoidance, ssthresh=" << state->ssthresh << "\n";
            hystartppEnded();
            return false;
        }
    }

    // "cwnd = cwnd + min(N, L * SMSS)", divided by CSS_GROWTH_DIVISOR in CSS; as in the
    // slow start of RFC 5681, only while the sender fills cwnd (Linux tcp_is_cwnd_limited())
    if (state->snd_effmss == 0 || state->snd_cwnd / state->snd_effmss < 2 * state->maxPacketsOut) {
        uint32_t increment = std::min(numBytesAcked, HYSTARTPP_L * state->snd_effmss);
        if (state->hystartppPhase == HYSTARTPP_CSS)
            increment /= HYSTARTPP_CSS_GROWTH_DIVISOR;
        state->snd_cwnd += increment;
        conn->emit(cwndSignal, state->snd_cwnd);
    }
    EV_INFO << "HyStart++: " << (state->hystartppPhase == HYSTARTPP_CSS ? "CSS" : "slow start") << ", cwnd=" << state->snd_cwnd << "\n";

    // "Keep track of the minimum observed RTT"
    simtime_t rtt = getAckRttSample(firstSeqAcked);
    if (rtt >= SIMTIME_ZERO) {
        if (state->hystartppCurrentRoundMinRtt < SIMTIME_ZERO || rtt < state->hystartppCurrentRoundMinRtt)
            state->hystartppCurrentRoundMinRtt = rtt;
        state->hystartppRttSampleCount++;
    }
    if (state->hystartppRttSampleCount < HYSTARTPP_N_RTT_SAMPLE || state->hystartppCurrentRoundMinRtt < SIMTIME_ZERO)
        return true;

    if (state->hystartppPhase == HYSTARTPP_SLOW_START) {
        if (state->hystartppLastRoundMinRtt < SIMTIME_ZERO)
            return true;
        // "RttThresh = max(MIN_RTT_THRESH, min(lastRoundMinRTT / MIN_RTT_DIVISOR, MAX_RTT_THRESH))"
        simtime_t rttThresh = std::max(SimTime(HYSTARTPP_MIN_RTT_THRESH), std::min(state->hystartppLastRoundMinRtt / HYSTARTPP_MIN_RTT_DIVISOR, SimTime(HYSTARTPP_MAX_RTT_THRESH)));
        if (state->hystartppCurrentRoundMinRtt >= state->hystartppLastRoundMinRtt + rttThresh) {
            state->hystartppCssBaselineMinRtt = state->hystartppCurrentRoundMinRtt;
            state->hystartppPhase = HYSTARTPP_CSS;
            state->hystartppCssRounds = 0;
            EV_INFO << "HyStart++: the minimum RTT of the round grew from " << state->hystartppLastRoundMinRtt << " to "
                    << state->hystartppCurrentRoundMinRtt << ", conservative slow start (CSS)\n";
        }
    }
    else if (state->hystartppCurrentRoundMinRtt < state->hystartppCssBaselineMinRtt) {
        // "the current round's minRTT drops below baseline (cssBaselineMinRtt) indicating
        // that slow start exit was spurious": "resume slow start including HyStart++"
        EV_INFO << "HyStart++: the minimum RTT of the round fell to " << state->hystartppCurrentRoundMinRtt << ", below "
                << state->hystartppCssBaselineMinRtt << ": the exit was spurious, slow start again\n";
        state->hystartppCssBaselineMinRtt = -1;
        state->hystartppPhase = HYSTARTPP_SLOW_START;
    }
    return true;
}

uint32_t TcpClassicAlgorithmBase::getBytesInFlight() const
{
    // Linux tcp_packets_in_flight(): packets_out - (sacked_out + lost_out) + retrans_out.
    // Without SACK, the duplicate ACKs set the SACKed marks (addInferredSack()).
    auto rexmitQueue = conn->getRexmitQueue();
    int64_t inFlight = (int64_t)(state->snd_max - conn->getDataSndUna()) - rexmitQueue->getSacked() - rexmitQueue->getLost() + rexmitQueue->getRetrans();
    return inFlight < 0 ? 0 : inFlight;
}

} // namespace tcp
} // namespace inet

