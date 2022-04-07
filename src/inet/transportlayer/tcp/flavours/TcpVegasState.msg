//
// Copyright (C) 2013 Maria Fernandez, Carlos Calafate, Juan-Carlos Cano and Pietro Manzoni
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.transportlayer.tcp.flavours.TcpBaseAlgState;
import inet.transportlayer.tcp.flavours.TcpSegmentTransmitInfoList;

namespace inet::tcp;

///
/// State variables for TcpVegas.
///
struct TcpVegasStateVariables extends TcpBaseAlgStateVariables
{
    @descriptor(readonly);

    uint32_t v_recoverypoint = 0;
    simtime_t v_cwnd_changed = 0; // last time cwnd changes because of a rtx.

    simtime_t v_baseRTT = SIMTIME_MAX;
    simtime_t v_sumRTT = SIMTIME_ZERO; // sum of rtt's measured within one RTT
    int v_cntRTT = 0; // # of rtt's measured within one RTT
    uint32_t v_begseq = 0; // register next pkt to be sent,for rtt calculation in receivedDataAck
    simtime_t v_begtime = 0; // register time for rtt calculation

    simtime_t v_rtt_timeout = 1000.0; // vegas fine-grained timeout
    simtime_t v_sa; // average for vegas fine-grained timeout
    simtime_t v_sd; // deviation for vegas fine-grained timeout

    TcpSegmentTransmitInfoList regions;

    uint32_t ssthresh = 65536; ///< slow start threshold

    bool v_inc_flag = true; // for slow start: "exponential growth only every other RTT"
    bool v_incr_ss = false; // to control no incr. cwnd if during slowstart ssthresh has been exceeded before the rtt is over
    int32_t v_incr = 0; // incr/decr
    uint32_t v_worried = 0; // pkts a to retransmit due to vegas fine-grained timeout
};

cplusplus(TcpVegasStateVariables) {{
  public:
    virtual std::string str() const override;
    virtual std::string detailedInfo() const override;
}}

