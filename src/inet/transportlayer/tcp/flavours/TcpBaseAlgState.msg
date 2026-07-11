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
/// State variables for TcpBaseAlg.
///
struct TcpBaseAlgStateVariables extends TcpStateVariables
{
    @descriptor(readonly);

    /// retransmit count
    //@{
    int rexmit_count = 0; ///< number of retransmissions (=1 after first rexmit)
    simtime_t rexmit_timeout = 3.0; ///< current retransmission timeout (aka RTO)
    simtime_t min_rto = 1.0; ///< lower bound on RTO (from the minRto parameter)
    simtime_t max_rto = 240.0; ///< upper bound on RTO (from the maxRto parameter)
    //@}

    /// persist factor
    //@{
    unsigned int persist_factor = 0; ///< factor needed for simplified PERSIST timer calculation
    simtime_t persist_timeout = 5.0; ///< current persist timeout
    //@}

    /// keepalive (RFC 1122 4.2.3.6)
    //@{
    bool keepalive_enabled = false; ///< whether keepalive probing is active on this connection
    simtime_t keepalive_idle_time = 7200; ///< idle time before the first probe
    simtime_t keepalive_interval = 75; ///< interval between probes
    int keepalive_max_probes = 9; ///< number of unanswered probes before abort
    int keepalive_probes_sent = 0; ///< probes sent in the current idle episode
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
    //@}

    /// number of RTOs
    //@{
    uint32_t numRtos = 0; ///< total number of RTOs
    //@}

    /// RFC 3782 variables
    //@{
    uint32_t recover = iss; ///< recover (RFC 3782)
    bool firstPartialACK = false; ///< first partial acknowledgement (RFC 3782)
    //@}

    /// per-segment transmit times, shared by all flavours (RTT sampling, RACK/Eifel input)
    //@{
    TcpSegmentTransmitInfoList sentInfo;
    //@}
};

cplusplus(TcpBaseAlgStateVariables) {{
  public:
    virtual std::string str() const override;
    virtual std::string detailedInfo() const override;
}}

