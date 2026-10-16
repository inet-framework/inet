//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.transportlayer.tcp.TcpConnectionState;
import inet.transportlayer.tcp.flavours.TcpSegmentTransmitInfoList;

cplusplus {{
    #include "inet/transportlayer/tcp/TcpAlgorithm.h"
}}

namespace inet::tcp;

///
/// State variables for TcpAlgorithmBase.
///
struct TcpAlgorithmBaseStateVariables extends TcpStateVariables
{
    @descriptor(readonly);

    /// retransmit count
    //@{
    int rexmit_count = 0; ///< number of retransmissions (=1 after first rexmit)
    simtime_t rexmit_timeout = 3.0; ///< current retransmission timeout (aka RTO)
    //@}

    /// persist timer
    //@{
    simtime_t persist_timeout = 5.0; ///< the interval of the next zero-window probe
    //@}

    /// keepalive (RFC 9293 section 3.8.4)
    //@{
    bool keepalive_enabled = false; ///< keepalive probes on this connection
    simtime_t keepalive_idle_time = 7200; ///< the idle time before the first probe
    simtime_t keepalive_interval = 75; ///< the time between two probes
    int keepalive_max_probes = 9; ///< the probes without an answer, after which the connection aborts
    int keepalive_probes_sent = 0; ///< the probes of this idle time
    simtime_t keepalive_last_probe_time = 0; ///< the time of the last keepalive probe
    //@}

    /// congestion window
    //@{
    uint32_t snd_cwnd = 0; ///< congestion window
    //@}

    /// round-trip time measurements
    //@{
    uint32_t rtseq = 0; ///< starting sequence number of timed data
    simtime_t rtseq_sendtime = SIMTIME_ZERO; ///< time when rtseq was sent (0 if RTT measurement is not running)
    //@}

    /// round-trip time estimation (Jacobson's algorithm)
    //@{
    simtime_t srtt = 0; ///< smoothed round-trip time
    simtime_t rttvar = 3.0 / 4.0; ///< variance of round-trip time
    bool rttMeasured = false; ///< whether a round-trip time has been measured yet (RFC 6298 2.2 vs 2.3)
    //@}

    /// number of RTOs
    //@{
    uint32_t numRtos = 0; ///< total number of RTOs
    uint32_t zeroWindowProbesSent = 0; ///< cumulative zero-window (persist) probes
    //@}

    /// RFC 3782 variables
    //@{
    uint32_t recover = iss; ///< recover (RFC 3782)
    bool firstPartialACK = false; ///< first partial acknowledgement (RFC 3782)
    //@}

    /// per-segment transmit times, shared by all flavours (RTT sampling)
    //@{
    TcpSegmentTransmitInfoList sentInfo;
    //@}
};

cplusplus(TcpAlgorithmBaseStateVariables) {{
  public:
    virtual std::string str() const override;
    virtual std::string detailedInfo() const override;
}}

