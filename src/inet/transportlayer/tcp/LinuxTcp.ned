//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp;

//
// The ~Tcp module with the defaults of Linux 7.1 and its stock sysctls; ~Tcp
// has the defaults of the RFCs. Select it with:
//
//   **.tcp.typename = "LinuxTcp"
//
// It sets each parameter that selects a behavior of Linux, and the values of
// Linux where the RFCs give none. The packetdrill suite of inet-gpl compares
// it with Linux 7.1.3. The ticks of the RACK reordering timer are those of a
// kernel with HZ=250.
//
// @see ~Tcp, ~LegacyTcp
//
simple LinuxTcp extends Tcp
{
    parameters:
        @class(Tcp);
        // congestion control: tcp_congestion_control=cubic, TCP_INIT_CWND
        tcpAlgorithmClass = default("TcpCubic");
        hystartEnabled = default(true);
        initialWindow = default("rfc6928");
        // loss recovery: tcp_recovery=1, tcp_frto=2, tcp_reordering=3
        adaptiveReorderingEnabled = default(true);
        frtoEnabled = default(true);
        lossUndoEnabled = default(true);
        retainSackAfterRto = default(true);
        rackReoTimerGranularity = default(4ms);
        // timers: tcp_set_rto(), TCP_RTO_MIN, TCP_RTO_MAX, tcp_retries2, tcp_syn_retries,
        // tcp_syn_linear_timeouts, and a TIME_WAIT of 60 s (TCP_TIMEWAIT_LEN)
        seedRttFromHandshake = default(true);
        minRexmitTimeout = default(200ms);
        rtoVarianceFloor = default(true);
        maxRexmitTimeout = default(120s);
        maxRexmitCount = default(15);
        synRetries = default(6);
        synLinearTimeouts = default(4);
        maxPersistTimeout = default(120s);
        zeroWindowProbe = default("linux");
        msl = default(30s);
        // sending and receiving: the delayed ACK, Nagle, PSH and the receive window of Linux
        adaptiveDelayedAcks = default(true);
        minshallNagle = default(true);
        pushSegmentsOnWriteBoundary = default(true);
        windowAutoTuning = default(true);
        windowUpdateAfterRead = default(true);
        acceptBeyondBufferAtEmptyQueue = default(true);
        dropBeyondWindowEdge = default(true);
        // the checks of a segment and of an ICMP error, and the reset at an abort
        softIcmpErrorsAbortSetup = default(true);
        icmpSequenceCheck = default(true);
        tsRecentAfterAckCheck = default(true);
        rfc5961AckCheck = default(true);
        abortRstWithAck = default(true);
        abortOnDataAfterClose = default(true);
        // ECN: tcp_ecn=2, the passive ECN of Tcp, and the option of AccECN as Linux sends it
        accEcnOptionPolicy = default("linux");
        accEcnBareTimeWaitAck = default(true);
        ecnMarkAll = default(true);
        // TCP Fast Open: tcp_fastopen=1, tcp_fastopen_blackhole_timeout_sec=0
        fastopenClientEnabled = default(true);
        fastopenBlackholeTimeout = default(0s);
        fastopenExpOptionEnabled = default(true);
}
