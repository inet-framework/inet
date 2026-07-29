//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/TcpCubic.h"

#include <algorithm> // max
#include <cmath> // pow

#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/flavours/Rfc6582Recovery.h"
#include "inet/transportlayer/tcp/flavours/Rfc6675Recovery.h"

namespace inet {
namespace tcp {

Register_Class(TcpCubic);

// RTT samples needed before the delay-increase detector may fire.
static const uint32_t HYSTART_MIN_SAMPLES = 8;

// While the window is unchanged, Linux recomputes ca->cnt at most once per
// HZ/32; in between it reuses the cached value. Keeping that rate limit
// matters: with a fast ACK clock the recomputation would otherwise happen many
// times per round trip and let the curve drift away from the kernel's.
static const double CNT_RECOMPUTE_INTERVAL = 1.0 / 32;

// The Reno-emulation estimator counts in units of beta_scale/8 segments per
// increment. Linux derives that scale from its fixed beta of 717/1024; the
// integer division is part of the result (it evaluates to 15), so it is spelled
// out the same way here.
static const uint32_t BETA_SCALE = 8 * (1024 + 717) / 3 / (1024 - 717);

TcpCubic::TcpCubic() : TcpAlgorithmBase(),
    state((TcpCubicStateVariables *&)TcpAlgorithm::state)
{
}

TcpCubic::~TcpCubic()
{
    delete recovery;
}

void TcpCubic::initialize()
{
    TcpAlgorithmBase::initialize();

    state->ssthresh = conn->getTcpMain()->par("initialSsthresh");

    state->cubic_beta = conn->getTcpMain()->par("cubicBeta");
    state->cubic_c = conn->getTcpMain()->par("cubicC");
    state->cubic_fast_convergence = conn->getTcpMain()->par("cubicFastConvergence");
    state->cubic_tcp_friendliness = conn->getTcpMain()->par("cubicTcpFriendliness");
    state->cubic_delta = conn->getTcpMain()->par("cubicDelta");
    state->cubic_cnt_clamp = conn->getTcpMain()->par("cubicCntClamp");
    state->hystart_enabled = conn->getTcpMain()->par("hystartEnabled");
    state->hystart_detect = conn->getTcpMain()->par("hystartDetect");
    state->hystart_low_window = conn->getTcpMain()->par("hystartLowWindow");
    state->hystart_ack_delta = conn->getTcpMain()->par("hystartAckDelta");
    state->hystart_delay_min = conn->getTcpMain()->par("hystartDelayMin");
    state->hystart_delay_max = conn->getTcpMain()->par("hystartDelayMax");

    cubicReset();
}

ITcpRecovery *TcpCubic::createRecovery()
{
    // SACK is orthogonal to the congestion control flavour: when the connection
    // negotiated SACK, loss recovery must be the RFC 6675 scoreboard-based one,
    // because the SACK receive path requires an Rfc6675Recovery.
    if (state->sack_enabled)
        return new Rfc6675Recovery(state, conn);
    else
        return new Rfc6582Recovery(state, conn);
}

void TcpCubic::established(bool active)
{
    TcpAlgorithmBase::established(active);

    // getRecovery() may already have created the strategy for a TCP Fast Open
    // server exchanging data in SYN_RCVD -- keep that instance, because its SACK
    // scoreboard context must survive the transition to ESTABLISHED.
    if (recovery == nullptr)
        recovery = createRecovery();
}

ITcpRecovery *TcpCubic::getRecovery()
{
    // TCP Fast Open server in SYN_RCVD: data flows before established() runs, so
    // the strategy has to be created on demand. SACK negotiation is final by
    // then, so this makes the same choice established() would.
    if (recovery == nullptr && state->fastopenAccelerated)
        recovery = createRecovery();
    return recovery;
}

void TcpCubic::cubicReset()
{
    state->cubic_last_max_cwnd = 0;
    state->cubic_origin_point = 0;
    state->cubic_K = 0;
    state->cubic_delay_min = -1;
    state->cubic_cnt = 0;
    state->cubic_last_cwnd = 0;
    state->cubic_last_time = -1;
    state->cubic_ack_cnt = 0;
    state->cubic_tcp_cwnd = 0;
    state->hystart_found = false;
}

void TcpCubic::hystartReset()
{
    state->hystart_round_start = state->hystart_last_ack = simTime();
    state->hystart_end_seq = state->snd_max;
    state->hystart_curr_rtt = -1;
    state->hystart_sample_cnt = 0;
}

void TcpCubic::processRexmitTimer(TcpEventCode& event)
{
    TcpAlgorithmBase::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;

    state->ssthresh = calculateSsthresh(getBytesInFlight());
    conn->emit(ssthreshSignal, state->ssthresh);

    state->snd_cwnd = state->snd_effmss;
    conn->emit(cwndSignal, state->snd_cwnd);

    EV_INFO << "Begin Slow Start: resetting cwnd to " << state->snd_cwnd
            << ", ssthresh=" << state->ssthresh << "\n";

    state->afterRto = true;
    conn->markOutstandingLostOnRto();
    conn->retransmitOneSegment(true);

    // Linux cubictcp_state(TCP_CA_Loss): a timeout invalidates the curve and the
    // W_max memory, and slow start begins again, so HyStart starts a new round.
    cubicReset();
    hystartReset();
}

void TcpCubic::receivedAckForAlreadyAckedData(const TcpHeader *tcpHeader, uint32_t payloadLength)
{
    TcpAlgorithmBase::receivedAckForAlreadyAckedData(tcpHeader, payloadLength);

    if (recovery->isDuplicateAck(tcpHeader, payloadLength)) {
        if (!state->lossRecovery) {
            state->dupacks++;
            conn->emit(dupAcksSignal, state->dupacks);
        }
        receivedDuplicateAck();
    }
    else {
        state->dupacks = 0;
        conn->emit(dupAcksSignal, state->dupacks);
    }
}

void TcpCubic::receivedAckForUnackedData(uint32_t firstSeqAcked)
{
    TcpAlgorithmBase::receivedAckForUnackedData(firstSeqAcked);

    uint32_t numBytesAcked = state->snd_una - firstSeqAcked;
    uint32_t numSegmentsAcked = numBytesAcked / state->snd_effmss;

    processAckRttSample(firstSeqAcked);

    if (state->lossRecovery)
        recovery->receivedAckForUnackedData(numBytesAcked);
    else if (state->snd_cwnd < state->ssthresh)
        slowStart(numSegmentsAcked);
    else if (numSegmentsAcked > 0)
        congestionAvoidance(numSegmentsAcked);

    sendData(false);
    ensureRexmitTimerArmed();
}

void TcpCubic::receivedDuplicateAck()
{
    recovery->receivedDuplicateAck();
}

uint32_t TcpCubic::getBytesInFlight() const
{
    auto rexmitQueue = conn->getRexmitQueue();
    int64_t sentSize = state->snd_max - conn->getDataSndUna();
    int64_t in_flight = sentSize - rexmitQueue->getSacked() - rexmitQueue->getLost() + rexmitQueue->getRetrans();
    if (in_flight < 0)
        in_flight = 0;
    conn->emit(bytesInFlightSignal, in_flight);
    return in_flight;
}

void TcpCubic::slowStart(uint32_t segmentsAcked)
{
    // Grow by the number of segments this ACK acknowledged (RFC 3465 byte
    // counting) rather than by one SMSS per ACK, so that a delayed-ACK receiver
    // does not halve the slow-start rate. The cwnd-limited gate (Linux
    // tcp_is_cwnd_limited) holds the window back while the sender is not
    // actually filling it -- an application-limited flow must not inflate cwnd
    // it has never used.
    if (state->snd_effmss == 0 || (state->snd_cwnd / state->snd_effmss) < 2 * state->maxPacketsOut) {
        state->snd_cwnd += segmentsAcked * state->snd_effmss;
        conn->emit(cwndSignal, state->snd_cwnd);
    }

    EV_INFO << "Slow start: cwnd=" << state->snd_cwnd << " ssthresh=" << state->ssthresh << "\n";
}

void TcpCubic::congestionAvoidance(uint32_t segmentsAcked)
{
    uint32_t cnt = cubicUpdate(segmentsAcked);

    // Linux tcp_cong_avoid_ai. Credit accumulated while cnt was larger is spent
    // first (one SMSS, counter cleared), and only then do this ACK's segments
    // accumulate, with every further whole multiple of cnt buying one more SMSS.
    if (state->cubic_cwnd_cnt >= cnt) {
        state->cubic_cwnd_cnt = 0;
        state->snd_cwnd += state->snd_effmss;
        conn->emit(cwndSignal, state->snd_cwnd);
        EV_INFO << "Congestion avoidance: cwnd=" << state->snd_cwnd << "\n";
    }

    state->cubic_cwnd_cnt += segmentsAcked;

    if (state->cubic_cwnd_cnt >= cnt) {
        uint32_t increments = state->cubic_cwnd_cnt / cnt;
        state->cubic_cwnd_cnt -= increments * cnt;
        state->snd_cwnd += increments * state->snd_effmss;
        conn->emit(cwndSignal, state->snd_cwnd);
        EV_INFO << "Congestion avoidance: cwnd=" << state->snd_cwnd << "\n";
    }
    else
        EV_INFO << "Congestion avoidance: " << state->cubic_cwnd_cnt << " of " << cnt
                << " segments acked towards the next increment\n";
}

uint32_t TcpCubic::cubicUpdate(uint32_t segmentsAcked)
{
    uint32_t segCwnd = state->snd_cwnd / state->snd_effmss;

    // Counted even when the recomputation below is skipped, so no acked segment
    // is lost to the Reno-emulation estimator.
    state->cubic_ack_cnt += segmentsAcked;

    if (state->cubic_last_cwnd == segCwnd && state->cubic_last_time >= SIMTIME_ZERO
        && simTime() - state->cubic_last_time <= CNT_RECOMPUTE_INTERVAL)
        return std::max(state->cubic_cnt, 2u);

    state->cubic_last_cwnd = segCwnd;
    state->cubic_last_time = simTime();

    if (state->cubic_epoch_start == -1) {
        // A new epoch begins where the last window reduction left off.
        state->cubic_epoch_start = simTime();
        state->cubic_ack_cnt = segmentsAcked;
        state->cubic_tcp_cwnd = segCwnd;

        if (state->cubic_last_max_cwnd <= segCwnd) {
            // Already at or above the last known W_max: the curve starts here
            // and only probes upwards.
            state->cubic_K = 0.0;
            state->cubic_origin_point = segCwnd;
        }
        else {
            // K is the time the curve needs to climb from the current window
            // back to W_max, i.e. cbrt((W_max - cwnd) / C).
            state->cubic_K = std::pow((state->cubic_last_max_cwnd - segCwnd) / state->cubic_c, 1 / 3.);
            state->cubic_origin_point = state->cubic_last_max_cwnd;
        }
    }

    // Aim one min-RTT ahead: the window computed now is the one that should be
    // in effect when the next round of ACKs comes back.
    double t = (simTime() + state->cubic_delay_min - state->cubic_epoch_start).dbl();
    double offs = (t < state->cubic_K) ? state->cubic_K - t : t - state->cubic_K;
    uint32_t delta = state->cubic_c * std::pow(offs, 3);
    uint32_t target = (t < state->cubic_K) ? state->cubic_origin_point - delta : state->cubic_origin_point + delta;

    // Turn the window target into an ACK count: growing by one SMSS every
    // cwnd/(target-cwnd) acked segments traces the curve without per-ACK
    // floating point.
    uint32_t cnt;
    if (target > segCwnd)
        cnt = segCwnd / (target - segCwnd);
    else
        cnt = 100 * segCwnd; // beyond the target: grow only marginally

    // Before the first loss there is no W_max to aim at, so cap the count to
    // keep the window moving.
    if (state->cubic_last_max_cwnd == 0 && cnt > state->cubic_cnt_clamp)
        cnt = state->cubic_cnt_clamp;

    if (state->cubic_tcp_friendliness) {
        // Track the window an AIMD(1, beta) Reno flow would have reached and
        // never grow slower than it. This is the binding term just after an RTO,
        // where W_max was cleared and the curve alone would crawl.
        delta = (segCwnd * BETA_SCALE) >> 3;
        while (delta > 0 && state->cubic_ack_cnt > delta) {
            state->cubic_ack_cnt -= delta;
            state->cubic_tcp_cwnd++;
        }

        if (state->cubic_tcp_cwnd > segCwnd) {
            uint32_t maxCnt = segCwnd / (state->cubic_tcp_cwnd - segCwnd);
            if (cnt > maxCnt)
                cnt = maxCnt;
        }
    }

    // At most one SMSS per two acked segments, i.e. at most 1.5x per RTT.
    state->cubic_cnt = std::max(cnt, 2u);
    return state->cubic_cnt;
}

void TcpCubic::processAckRttSample(uint32_t firstSeqAcked)
{
    // Linux drives cubictcp_acked() from pkts_acked(), which gets the RTT of the
    // ACK being processed: tcp_clean_rtx_queue times the first newly acknowledged
    // segment against now (ack_sample::rtt_us), so EVERY ACK that advances snd_una
    // yields a sample. That per-ACK raw value is what HyStart needs -- both its
    // detectors count and compare individual samples, and the smoothed estimate
    // (which moves only an eighth of the way per ACK) can neither be counted
    // per-ACK nor rise fast enough to cross a threshold set 12.5% above the
    // connection minimum. Deliberately kept separate from the srtt/RTO estimator,
    // which stays on its own once-per-RTT schedule.
    const TcpSegmentTransmitInfoList::Item *sent = state->sentInfo.get(firstSeqAcked);
    if (sent == nullptr)
        return;
    // Karn's algorithm: a retransmitted segment cannot be timed, because there is
    // no telling which copy this ACK answers. Linux discards the sample for the
    // same reason (tcp_clean_rtx_queue only times !sacked_retrans segments when
    // the timestamp option is not available to disambiguate).
    if (sent->getTransmitCount() != 1)
        return;
    processRttSample(simTime() - sent->getFirstSentTime());
}

void TcpCubic::processRttSample(const simtime_t& rtt)
{
    // Right after a window reduction the samples still describe the old, larger
    // window, so let the connection settle before trusting them.
    if (state->cubic_epoch_start != -1 && simTime() - state->cubic_epoch_start < state->cubic_delta)
        return;

    if (state->cubic_delay_min == -1 || state->cubic_delay_min > rtt)
        state->cubic_delay_min = rtt;

    // HyStart only acts in slow start, and only once the window is large enough
    // for the detectors to be meaningful.
    if (state->hystart_enabled && state->snd_cwnd <= state->ssthresh
        && state->snd_cwnd >= state->hystart_low_window * state->snd_effmss)
        hystartUpdate(rtt);
}

void TcpCubic::hystartUpdate(const simtime_t& delay)
{
    if (state->hystart_found)
        return;

    // A round ends when everything that was in flight when it began has been
    // acknowledged. Linux opens the new round here, at the TOP of hystart_update,
    // and the placement is load-bearing: the very ACK that closes a round also
    // provides the new round's first RTT sample, so resetting afterwards (from the
    // slow-start path, which runs later in the ACK's processing) would throw that
    // sample away and delay every delay check by one ACK.
    if (seqGreater(state->snd_una, state->hystart_end_seq))
        hystartReset();

    simtime_t now = simTime();

    // ACK-train detector: while ACKs keep arriving back to back, the train's
    // length measures how much of the path is already filled; once it spans the
    // minimum RTT, the pipe is full.
    if (now - state->hystart_last_ack <= state->hystart_ack_delta) {
        state->hystart_last_ack = now;
        if (now - state->hystart_round_start > state->cubic_delay_min
            && (state->hystart_detect & HYSTART_ACK_TRAIN))
            state->hystart_found = true;
    }

    // Delay-increase detector: once enough samples are in, a round whose
    // minimum RTT sits clearly above the connection minimum means a queue is
    // building up ahead. The round minimum tracks EVERY sample, including the
    // ones after the count is full -- Linux commit b344579ca847 ("tcp_cubic: fix
    // spurious HYSTART_DELAY exit upon drop in min RTT") moved this out of the
    // counting branch precisely so that a late sample which lowers the minimum is
    // still taken into account, instead of comparing a stale round minimum
    // against a delay_min the same ACK just pushed down.
    if (state->hystart_curr_rtt == -1 || state->hystart_curr_rtt > delay)
        state->hystart_curr_rtt = delay;

    if (state->hystart_sample_cnt < HYSTART_MIN_SAMPLES)
        ++state->hystart_sample_cnt;
    else if (state->hystart_curr_rtt > state->cubic_delay_min + hystartDelayThresh(state->cubic_delay_min / 8)
             && (state->hystart_detect & HYSTART_DELAY))
        state->hystart_found = true;

    if (state->hystart_found) {
        // Leave slow start at the current window instead of overshooting into loss.
        EV_INFO << "HyStart: exiting slow start, ssthresh=" << state->snd_cwnd << "\n";
        state->ssthresh = state->snd_cwnd;
    }
}

simtime_t TcpCubic::hystartDelayThresh(const simtime_t& t) const
{
    if (t > state->hystart_delay_max)
        return state->hystart_delay_max;
    if (t < state->hystart_delay_min)
        return state->hystart_delay_min;
    return t;
}

uint32_t TcpCubic::calculateSsthresh(uint32_t bytesInFlight)
{
    uint32_t segCwnd = state->snd_cwnd / state->snd_effmss;

    EV_DETAIL << "Loss at cwnd=" << segCwnd << " segments, in flight="
              << bytesInFlight / state->snd_effmss << " segments\n";

    // Fast convergence (RFC 9438 section 4.7): a flow that lost before reaching
    // the previous W_max is facing a new competitor, so it gives up a little
    // more of the window to let that competitor grow.
    if (segCwnd < state->cubic_last_max_cwnd && state->cubic_fast_convergence)
        state->cubic_last_max_cwnd = (segCwnd * (1 + state->cubic_beta)) / 2;
    else
        state->cubic_last_max_cwnd = segCwnd;

    state->cubic_epoch_start = -1; // the epoch ends with the reduction
    state->cubic_last_time = -1; // every window reduction forces a cnt recomputation

    return std::max(static_cast<uint32_t>(segCwnd * state->cubic_beta), 2u) * state->snd_effmss;
}

} // namespace tcp
} // namespace inet
