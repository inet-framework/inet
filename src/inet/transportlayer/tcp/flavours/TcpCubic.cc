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

TcpCubic::TcpCubic() : TcpClassicAlgorithmBase(),
    state((TcpCubicStateVariables *&)TcpAlgorithm::state)
{
}

void TcpCubic::initialize()
{
    TcpClassicAlgorithmBase::initialize();

    state->cubic_beta = conn->getTcpMain()->par("cubicBeta");
    state->cubic_c = conn->getTcpMain()->par("cubicC");
    state->cubic_fast_convergence = conn->getTcpMain()->par("cubicFastConvergence");
    state->cubic_tcp_friendliness = conn->getTcpMain()->par("cubicTcpFriendliness");
    state->cubic_delta = conn->getTcpMain()->par("cubicDelta");
    state->cubic_cnt_clamp = conn->getTcpMain()->par("cubicCntClamp");

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
}

void TcpCubic::processRexmitTimer(TcpEventCode& event)
{
    TcpClassicAlgorithmBase::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;

    // Linux cubictcp_state(TCP_CA_Loss): a timeout invalidates the curve and the
    // W_max memory, and slow start begins again.
    cubicReset();
}

void TcpCubic::receivedAckForUnackedData(uint32_t firstSeqAcked)
{
    TcpAlgorithmBase::receivedAckForUnackedData(firstSeqAcked);

    uint32_t numBytesAcked = state->snd_una - firstSeqAcked;
    uint32_t numSegmentsAcked = numBytesAcked / state->snd_effmss;

    processAckRttSample(firstSeqAcked);

    if (state->lossRecovery)
        recovery->receivedAckForUnackedData(numBytesAcked);
    else if (processEce())
        ; // an ECN-Echo reaction replaces this ACK's window growth (RFC 3168)
    else if (state->snd_cwnd < state->ssthresh)
        slowStart(numSegmentsAcked);
    else if (numSegmentsAcked > 0)
        congestionAvoidance(numSegmentsAcked);

    sendData(false);
    ensureRexmitTimerArmed();
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
    // yields a sample, and the minimum of these samples is the delay that the
    // cubic curve adds to the time since the epoch started. Deliberately kept
    // separate from the srtt/RTO estimator, which stays on its own once-per-RTT
    // schedule.
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
