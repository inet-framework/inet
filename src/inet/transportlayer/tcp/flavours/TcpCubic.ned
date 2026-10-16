//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp.flavours;

//
// CUBIC (RFC 9438), as Linux tcp_cubic.c, with its slow start (HyStart).
//
simple TcpCubic extends TcpAlgorithm
{
    parameters:
        @class(TcpCubic);
        double beta = default(0.7); // multiplicative-decrease factor applied to cwnd on loss (RFC 9438 section 4.6). Note that the TCP-friendliness estimator keeps the kernel's fixed scale (derived from beta=717/1024), so changing this only alters the decrease itself
        double c = default(0.4); // scaling constant C of the cubic window-growth curve (RFC 9438 section 4.2)
        bool fastConvergence = default(true); // on a loss below the previous W_max, release extra window so a newly arrived competing flow can grow (RFC 9438 section 4.7)
        bool tcpFriendliness = default(true); // never grow slower than an AIMD(1, beta) Reno flow would (RFC 9438 section 4.3)
        // Linux behaviors (no RFC)
        int cntClamp = default(20); // cap on the per-increment ACK count while no W_max is known yet (before the first loss)
        double delta @unit(s) = default(10ms); // settling time after a window reduction, during which RTT samples are ignored
        bool hystartEnabled = default(false); // HyStart: leave slow start when the path appears full, instead of overshooting into loss; ~LinuxTcp and ~LegacyTcp turn it on
        double hystartAckDelta @unit(s) = default(2ms); // largest ACK spacing still counted as part of an ACK train
        double hystartDelayMin @unit(s) = default(4ms); // lower clamp on the HyStart delay-increase threshold (an eighth of the minimum RTT)
        double hystartDelayMax @unit(s) = default(16ms); // upper clamp on the same threshold (Linux HYSTART_DELAY_MAX)
        int hystartDetect = default(3); // HyStart detectors, as a bitmask: 1 = ACK train, 2 = delay increase, 3 = both
        int hystartLowWindow = default(16); // smallest cwnd (in segments) at which HyStart starts looking for the exit point
}
