//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/TcpCubic.h"

#include <algorithm> // min, max
#include <cmath> // cbrt

#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/flavours/TcpSegmentTransmitInfoList.h"

namespace inet {
namespace tcp {

Register_Class(TcpCubic);

// HyStart constants (Linux tcp_cubic.c); seconds (global simtime_t is forbidden)
static const int HYSTART_MIN_SAMPLES = 8;
static const double HYSTART_DELAY_MIN = 0.004; // 4 ms
static const double HYSTART_DELAY_MAX = 0.016; // 16 ms

TcpCubic::TcpCubic() : TcpReno(),
    state((TcpCubicStateVariables *&)TcpAlgorithm::state)
{
}

void TcpCubic::initialize()
{
    TcpReno::initialize();

    state->cubic_beta = conn->getTcpMain()->par("cubicBeta");
    state->cubic_c = conn->getTcpMain()->par("cubicC");
    state->cubic_fast_convergence = conn->getTcpMain()->par("cubicFastConvergence");
    state->cubic_tcp_friendliness = conn->getTcpMain()->par("cubicTcpFriendliness");
    state->hystart_enabled = conn->getTcpMain()->par("hystartEnabled");
    state->hystart_detect = conn->getTcpMain()->par("hystartDetect");
    state->hystart_low_window = conn->getTcpMain()->par("hystartLowWindow");
    state->hystart_ack_delta = conn->getTcpMain()->par("hystartAckDelta");

    cubicReset();
}

void TcpCubic::cubicReset()
{
    // Linux bictcp_reset: forget the epoch and the fast-convergence memory.
    state->cubic_last_max_cwnd = 0;
    state->cubic_epoch_start = 0;
    state->cubic_origin_point = 0;
    state->cubic_K = 0;
    state->cubic_ack_cnt = 0;
    state->cubic_tcp_cwnd = 0;
    state->cubic_cnt = 0;
    state->cubic_cwnd_cnt = 0;
    state->cubic_delay_min = 0;
    state->hystart_found = false;
}

void TcpCubic::hystartReset()
{
    // Linux bictcp_hystart_reset: start a fresh RTT round.
    state->hystart_round_start = state->hystart_last_ack = simTime();
    state->hystart_end_seq = state->snd_max;
    state->hystart_curr_rtt = 0; // 0 = unset
    state->hystart_sample_cnt = 0;
}

void TcpCubic::established(bool active)
{
    TcpReno::established(active);
    cubicReset();
    if (state->hystart_enabled)
        hystartReset();
}

void TcpCubic::recalculateSlowStartThreshold()
{
    // Linux cubictcp_recalc_ssthresh: end the epoch, apply fast convergence, and
    // set ssthresh = beta * cwnd (at least 2 SMSS).
    state->cubic_epoch_start = 0; // end of the current epoch

    uint32_t cwnd = state->snd_cwnd;
    if (cwnd < state->cubic_last_max_cwnd && state->cubic_fast_convergence)
        state->cubic_last_max_cwnd = (uint32_t)(cwnd * (1.0 + state->cubic_beta) / 2.0);
    else
        state->cubic_last_max_cwnd = cwnd;

    state->ssthresh = std::max((uint32_t)(cwnd * state->cubic_beta), 2 * state->snd_mss);

    conn->emit(ssthreshSignal, state->ssthresh);
}

void TcpCubic::hystartUpdate(simtime_t delay)
{
    // Linux hystart_update: called during slow start to find the exit point.
    if (seqGreater(state->snd_una, state->hystart_end_seq))
        hystartReset();

    if (state->hystart_found)
        return;

    // ACK-train detector: if ACKs arrive in a train longer than half the min RTT,
    // the pipe is considered full.
    if (state->hystart_detect & 1) {
        simtime_t now = simTime();
        if (now - state->hystart_last_ack <= state->hystart_ack_delta) {
            state->hystart_last_ack = now;
            if (now - state->hystart_round_start > state->cubic_delay_min / 2) {
                state->hystart_found = true;
                state->ssthresh = state->snd_cwnd;
                conn->emit(ssthreshSignal, state->ssthresh);
                EV_INFO << "HyStart: ACK-train exit, ssthresh set to " << state->ssthresh << "\n";
            }
        }
    }

    // delay-increase detector: if the round's minimum RTT rises above the
    // connection minimum by more than eta, the pipe is considered full.
    if (!state->hystart_found && (state->hystart_detect & 2)) {
        if (state->hystart_curr_rtt == 0 || delay < state->hystart_curr_rtt)
            state->hystart_curr_rtt = delay;
        if (++state->hystart_sample_cnt >= HYSTART_MIN_SAMPLES) {
            double eta = state->cubic_delay_min.dbl() / 8.0;
            if (eta < HYSTART_DELAY_MIN)
                eta = HYSTART_DELAY_MIN;
            if (eta > HYSTART_DELAY_MAX)
                eta = HYSTART_DELAY_MAX;
            if (state->hystart_curr_rtt > state->cubic_delay_min + eta) {
                state->hystart_found = true;
                state->ssthresh = state->snd_cwnd;
                conn->emit(ssthreshSignal, state->ssthresh);
                EV_INFO << "HyStart: delay-increase exit, ssthresh set to " << state->ssthresh << "\n";
            }
        }
    }
}

void TcpCubic::cubicUpdate(uint32_t ackedBytes)
{
    // Linux bictcp_update, computed in segment units.
    uint32_t mss = state->snd_mss;
    double cwndSeg = (double)state->snd_cwnd / mss;
    uint32_t ackedSeg = std::max((uint32_t)1, ackedBytes / mss);

    state->cubic_ack_cnt += ackedSeg;

    if (state->cubic_epoch_start == 0) {
        // start a new congestion-avoidance epoch
        state->cubic_epoch_start = simTime();
        state->cubic_ack_cnt = ackedSeg;
        state->cubic_tcp_cwnd = (uint32_t)cwndSeg;

        double lastMaxSeg = (double)state->cubic_last_max_cwnd / mss;
        if (state->cubic_last_max_cwnd <= state->snd_cwnd) {
            state->cubic_K = 0;
            state->cubic_origin_point = (uint32_t)cwndSeg;
        }
        else {
            state->cubic_K = std::cbrt((lastMaxSeg - cwndSeg) / state->cubic_c);
            state->cubic_origin_point = (uint32_t)lastMaxSeg;
        }
    }

    // t = time since epoch start + minimum RTT (seconds)
    double t = (simTime() - state->cubic_epoch_start).dbl() + state->cubic_delay_min.dbl();
    double dt = t - state->cubic_K;
    double target = (double)state->cubic_origin_point + state->cubic_c * dt * dt * dt;

    if (target > cwndSeg)
        state->cubic_cnt = (uint32_t)(cwndSeg / (target - cwndSeg));
    else
        state->cubic_cnt = 100 * (uint32_t)cwndSeg; // very slow growth beyond W_max

    // in the first epoch after a fresh reset, grow gently
    if (state->cubic_last_max_cwnd == 0 && state->cubic_cnt > 20)
        state->cubic_cnt = 20;

    // TCP friendliness: never grow slower than Reno would
    if (state->cubic_tcp_friendliness) {
        double beta = state->cubic_beta;
        double scale = 8.0 * (1.0 + beta) / (3.0 * (1.0 - beta));
        uint32_t delta = std::max((uint32_t)1, (uint32_t)(cwndSeg * scale / 8.0));
        while (state->cubic_ack_cnt > delta) {
            state->cubic_ack_cnt -= delta;
            state->cubic_tcp_cwnd++;
        }
        if (state->cubic_tcp_cwnd > (uint32_t)cwndSeg) {
            uint32_t d = state->cubic_tcp_cwnd - (uint32_t)cwndSeg;
            uint32_t max_cnt = (uint32_t)cwndSeg / d;
            if (state->cubic_cnt > max_cnt)
                state->cubic_cnt = max_cnt;
        }
    }

    if (state->cubic_cnt == 0)
        state->cubic_cnt = 1;
}

void TcpCubic::congestionAvoidanceAi(uint32_t ackedBytes)
{
    // Linux tcp_cong_avoid_ai: raise cwnd by one SMSS every cubic_cnt ACKs.
    uint32_t ackedSeg = std::max((uint32_t)1, ackedBytes / state->snd_mss);
    state->cubic_cwnd_cnt += ackedSeg;
    if (state->cubic_cwnd_cnt >= state->cubic_cnt) {
        state->cubic_cwnd_cnt = 0;
        state->snd_cwnd += state->snd_mss;
        conn->emit(cwndSignal, state->snd_cwnd);
    }
}

void TcpCubic::receivedDataAck(uint32_t firstSeqAcked)
{
    TcpTahoeRenoFamily::receivedDataAck(firstSeqAcked);

    // RTT sample for delay_min (Linux cubictcp_acked): only from segments that
    // were transmitted exactly once (Karn's rule).
    const TcpSegmentTransmitInfoList::Item *item = state->sentInfo.get(firstSeqAcked);
    if (item && item->getTransmitCount() == 1) {
        simtime_t rtt = simTime() - item->getFirstSentTime();
        if (rtt > 0 && (state->cubic_delay_min == 0 || rtt < state->cubic_delay_min))
            state->cubic_delay_min = rtt;

        // HyStart acts only during slow start and once the window is large enough
        if (state->hystart_enabled && !state->hystart_found
            && state->snd_cwnd < state->ssthresh
            && state->snd_cwnd / state->snd_mss >= (uint32_t)state->hystart_low_window)
        {
            hystartUpdate(rtt);
        }
    }

    if (state->dupacks >= state->dupthresh) {
        // Fast Recovery exit: deflate cwnd to ssthresh.
        EV_INFO << "Fast Recovery: setting cwnd to ssthresh=" << state->ssthresh << "\n";
        state->snd_cwnd = state->ssthresh;
        conn->emit(cwndSignal, state->snd_cwnd);
    }
    else {
        bool performGrowth = true;
        if (state->ect && state->gotEce) {
            // treat an ECN-Echo like a congestion signal, at most once per RTT,
            // using the cubic multiplicative decrease
            if (simTime() - state->eceReactionTime > state->srtt) {
                recalculateSlowStartThreshold();
                state->snd_cwnd = std::max(state->ssthresh, state->snd_mss);
                state->sndCwr = true;
                performGrowth = false;
                if (state->snd_cwnd == state->snd_mss)
                    restartRexmitTimer();
                state->eceReactionTime = simTime();
                conn->emit(cwndSignal, state->snd_cwnd);
            }
            state->gotEce = false;
        }
        if (performGrowth) {
            uint32_t bytesAcked = state->snd_una - firstSeqAcked;
            if (state->snd_cwnd < state->ssthresh) {
                // Slow start, Linux parity (tcp_slow_start + tcp_is_cwnd_limited):
                // grow cwnd by the number of segments this ACK newly acknowledged,
                // but only while the sender is cwnd-limited -- i.e. it actually
                // filled the window (cwnd < 2 * max_packets_out, both in segments).
                // An application-limited flow that never fills cwnd does not
                // inflate it. This replaces the old flat "+1 SMSS per ACK", which
                // both under-counted multi-segment ACKs and grew unconditionally.
                uint32_t segmentsAcked = bytesAcked / state->snd_mss;
                uint32_t cwndSegments = state->snd_cwnd / state->snd_mss;
                if (segmentsAcked > 0 && cwndSegments < 2 * state->maxPacketsOut) {
                    state->snd_cwnd += segmentsAcked * state->snd_mss;
                    conn->emit(cwndSignal, state->snd_cwnd);
                }
            }
            else {
                // cubic congestion avoidance
                cubicUpdate(bytesAcked);
                congestionAvoidanceAi(bytesAcked);
            }
        }
    }

    // RFC 3517 SACK loss recovery (inherited behavior, kept identical to TcpReno)
    if (state->sack_enabled && state->lossRecovery) {
        if (seqGE(state->snd_una, state->recoveryPoint)) {
            EV_INFO << "Loss Recovery terminated.\n";
            state->lossRecovery = false;
        }
        else {
            conn->setPipe();
            if (((int)state->snd_cwnd - (int)state->pipe) >= (int)state->snd_mss)
                conn->sendDataDuringLossRecoveryPhase(state->snd_cwnd);
        }
    }

    sendData(false);
}

void TcpCubic::processRexmitTimer(TcpEventCode& event)
{
    // TcpReno::processRexmitTimer calls the (virtual) recalculateSlowStartThreshold
    // above, so the cubic beta decrease is applied; it then sets cwnd = 1 SMSS.
    TcpReno::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;

    // Linux cubictcp_state(TCP_CA_Loss): drop the epoch and the fast-convergence
    // memory, and restart HyStart.
    cubicReset();
    if (state->hystart_enabled)
        hystartReset();
}

} // namespace tcp
} // namespace inet
