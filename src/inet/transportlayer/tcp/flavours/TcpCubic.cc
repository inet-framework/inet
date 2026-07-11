//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/TcpCubic.h"

#include <algorithm> // min, max

#include "inet/transportlayer/tcp/Tcp.h"

namespace inet {
namespace tcp {

Register_Class(TcpCubic);

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
