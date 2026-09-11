//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.transportlayer.tcp_common.TcpHeader;
import inet.transportlayer.tcp.flavours.TcpBaseAlgState;

namespace inet::tcp;

///
/// State variables for TcpTahoeRenoFamily.
///
struct TcpTahoeRenoFamilyStateVariables extends TcpBaseAlgStateVariables
{
    @descriptor(readonly);

    uint32_t ssthresh; ///< slow start threshold
};

cplusplus(TcpTahoeRenoFamilyStateVariables) {{
  public:
    virtual std::string str() const override;
    virtual std::string detailedInfo() const override;
    virtual void setSendQueueLimit(uint32_t newLimit);
}}

