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
/// State variables for TcpWestwood.
///
struct TcpWestwoodStateVariables extends TcpBaseAlgStateVariables
{
    @descriptor(readonly);

    uint32_t ssthresh = 65535; ///< slow start threshold

    simtime_t w_RTTmin = SIMTIME_MAX; // min RTT
    double w_a = 1.0; // threshold reduction factor for ssthresh calculation

    simtime_t w_lastAckTime = SIMTIME_ZERO; // last received ack time

    double w_bwe = 0.0;
    double w_sample_bwe = 0.0;

    TcpSegmentTransmitInfoList regions;
};

cplusplus(TcpWestwoodStateVariables) {{
  public:
    virtual std::string str() const override;
    virtual std::string detailedInfo() const override;
}}

