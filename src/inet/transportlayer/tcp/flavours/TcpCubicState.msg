//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.transportlayer.tcp.flavours.TcpClassicAlgorithmBaseState;

namespace inet::tcp;

///
/// State variables for TcpCubic (RFC 9438), including the HyStart hybrid
/// slow-start exit. The field names follow Linux's struct bictcp so the
/// implementation can be read next to tcp_cubic.c.
///
/// Window quantities are kept in SEGMENTS (not bytes): CUBIC's curve, its
/// Reno-emulation estimate and the resulting ACK counter are all defined in
/// packets, and rounding the window down to whole segments once, up front,
/// keeps the integer arithmetic below identical to the kernel's.
///
struct TcpCubicStateVariables extends TcpClassicAlgorithmBaseStateVariables
{
    @descriptor(readonly);

    /// CUBIC curve (Linux struct bictcp)
    //@{
    uint32_t cubic_last_max_cwnd = 0; ///< W_max: window (segments) where the last loss occurred
    uint32_t cubic_origin_point = 0; ///< origin of the cubic curve (segments)
    double cubic_K = 0; ///< seconds from the epoch start to W_max
    simtime_t cubic_epoch_start = -1; ///< start of the current congestion-avoidance epoch; -1 = no epoch running
    simtime_t cubic_delay_min = -1; ///< smallest RTT seen since the last reset; -1 = no sample yet
    //@}

    /// cwnd increment pacing (Linux tcp_cong_avoid_ai)
    //@{
    uint32_t cubic_cnt = 0; ///< ACKs (in segments) required per 1-SMSS increase
    uint32_t cubic_cwnd_cnt = 0; ///< segments acked so far towards cubic_cnt
    uint32_t cubic_last_cwnd = 0; ///< window (segments) when cubic_cnt was last recomputed
    simtime_t cubic_last_time = -1; ///< when cubic_cnt was last recomputed; -1 = never
    //@}

    /// TCP friendliness: the window an AIMD(1, beta) Reno flow would have reached
    //@{
    uint32_t cubic_ack_cnt = 0; ///< segments acked in this epoch
    uint32_t cubic_tcp_cwnd = 0; ///< W_est (segments)
    //@}

    /// HyStart (hybrid slow start)
    //@{
    simtime_t hystart_round_start = -1; ///< start of the current RTT round; -1 = unset
    simtime_t hystart_last_ack = -1; ///< arrival of the last ACK, for the ACK-train detector; -1 = unset
    simtime_t hystart_curr_rtt = -1; ///< smallest RTT sampled in this round; -1 = no sample yet
    uint32_t hystart_end_seq = 0; ///< snd_max snapshot that closes the current round
    uint32_t hystart_sample_cnt = 0; ///< RTT samples taken in this round
    bool hystart_found = false; ///< the slow-start exit point has been detected
    //@}

    /// configuration, read from the NED parameters in TcpCubic::initialize()
    //@{
    double cubic_beta = 0.7; ///< multiplicative decrease factor
    double cubic_c = 0.4; ///< scaling constant of the cubic curve
    bool cubic_fast_convergence = true; ///< release more window when W_max shrinks
    bool cubic_tcp_friendliness = true; ///< never grow slower than Reno would
    simtime_t cubic_delta = 0.01; ///< settling time after a window reduction, during which RTT samples are ignored
    uint32_t cubic_cnt_clamp = 20; ///< cap on cubic_cnt while W_max is still unknown
    bool hystart_enabled = true; ///< look for a slow-start exit point
    int hystart_detect = 3; ///< bitmask of HystartDetect: 1 = ACK train, 2 = delay increase
    uint32_t hystart_low_window = 16; ///< smallest window (segments) at which HyStart acts
    simtime_t hystart_ack_delta = 0.002; ///< largest ACK spacing still counted as a train
    simtime_t hystart_delay_min = 0.004; ///< lower clamp on the delay-increase threshold
    simtime_t hystart_delay_max = 1; ///< upper clamp on the delay-increase threshold
    //@}
};
