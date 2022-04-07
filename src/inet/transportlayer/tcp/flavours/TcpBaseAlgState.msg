//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.transportlayer.tcp.TcpConnectionState;

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
    //@}

    /// persist factor
    //@{
    unsigned int persist_factor = 0; ///< factor needed for simplified PERSIST timer calculation
    simtime_t persist_timeout = 5.0; ///< current persist timeout
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
};

cplusplus(TcpBaseAlgStateVariables) {{
  public:
    virtual std::string str() const override;
    virtual std::string detailedInfo() const override;
}}

