//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.transportlayer.tcp_common.TcpHeader;
import inet.transportlayer.tcp.flavours.TcpAlgorithmBaseState;

namespace inet::tcp;

///
/// State variables for TcpClassicAlgorithmBase.
///
struct TcpClassicAlgorithmBaseStateVariables extends TcpAlgorithmBaseStateVariables
{
    @descriptor(readonly);

    uint32_t ssthresh; ///< slow start threshold
    int slowStartExit = 0; ///< slowStartExit: 0 = none, 1 = hystart (the hybrid slow start of Linux, TcpCubic only), 2 = hystartpp
    // HyStart++ (RFC 9406), the names of section 4.2; -1 stands for infinity
    int hystartppPhase = 0; ///< 0 = slow start, 1 = conservative slow start (CSS), 2 = ended
    uint32_t hystartppSsthresh = 0; ///< the initial ssthresh: a congestion response, which changes ssthresh, ends HyStart++
    uint32_t hystartppWindowEnd = 0; ///< windowEnd: the ACK of it ends the round
    simtime_t hystartppLastRoundMinRtt = -1; ///< lastRoundMinRTT
    simtime_t hystartppCurrentRoundMinRtt = -1; ///< currentRoundMinRTT
    simtime_t hystartppCssBaselineMinRtt = -1; ///< cssBaselineMinRtt
    uint32_t hystartppRttSampleCount = 0; ///< rttSampleCount
    uint32_t hystartppCssRounds = 0; ///< the ended rounds of CSS, the partial first one included
};

cplusplus(TcpClassicAlgorithmBaseStateVariables) {{
  public:
    virtual std::string str() const override;
    virtual std::string detailedInfo() const override;
    virtual void setSendQueueLimit(uint32_t newLimit);
}}

