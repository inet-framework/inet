//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.transportlayer.tcp.flavours.TcpTahoeRenoFamilyState;

namespace inet::tcp;

///
/// State variables for TcpCubic (RFC 9438 / Linux tcp_cubic.c), including HyStart.
///
struct TcpCubicStateVariables extends TcpTahoeRenoFamilyStateVariables
{
    @descriptor(readonly);

    // --- CUBIC core (Linux struct bictcp) ---
    uint32_t cubic_last_max_cwnd = 0;   // W_max, in bytes
    simtime_t cubic_epoch_start = 0;    // start of the current congestion-avoidance epoch; 0 = invalid
    double cubic_K = 0;                 // time to reach W_max from the epoch start, in seconds
    uint32_t cubic_origin_point = 0;    // W_max in segments (origin of the cubic function)
    uint32_t cubic_ack_cnt = 0;         // bytes acked in the current epoch (TCP-friendliness)
    uint32_t cubic_tcp_cwnd = 0;        // Reno-emulation cwnd estimate, in bytes
    uint32_t cubic_cnt = 0;             // ACKs (in MSS units) needed per 1-MSS cwnd increase
    uint32_t cubic_cwnd_cnt = 0;        // accumulator toward cubic_cnt
    simtime_t cubic_delay_min = 0;      // minimum RTT observed; 0 = unset

    // --- HyStart ---
    simtime_t hystart_round_start = 0;  // start time of the current RTT round
    simtime_t hystart_last_ack = 0;     // time of the last ACK (ACK-train detector)
    uint32_t hystart_end_seq = 0;       // snd_nxt snapshot marking the end of the round
    simtime_t hystart_curr_rtt = 0;     // minimum RTT sampled in this round; 0 = unset
    int hystart_sample_cnt = 0;         // number of RTT samples in this round
    bool hystart_found = false;         // slow-start exit point already found

    // --- configuration mirrors (read from NED parameters in initialize()) ---
    double cubic_beta = 0.700195;       // 717/1024, the CUBIC multiplicative-decrease factor
    double cubic_c = 0.4;               // the CUBIC scaling constant C
    bool cubic_fast_convergence = true;
    bool cubic_tcp_friendliness = true;
    bool hystart_enabled = true;
    int hystart_detect = 3;             // 1=ACK-train, 2=delay, 3=both
    int hystart_low_window = 16;        // minimum cwnd (segments) for HyStart to act
    simtime_t hystart_ack_delta = 0.002;// ACK-train inter-arrival threshold (s)
};
