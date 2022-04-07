//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.transportlayer.tcp.flavours.TcpTahoeRenoFamilyState;

namespace inet::tcp;

///
/// State variables for DcTcpFamily.
///
struct DcTcpFamilyStateVariables extends TcpTahoeRenoFamilyStateVariables
{
    @descriptor(readonly);

    // DCTCP
    bool dctcp_ce = false;
    uint32_t dctcp_windEnd = snd_una;
    uint32_t dctcp_bytesAcked = 0;
    uint32_t dctcp_bytesMarked = 0; // amount of bytes marked
    double dctcp_alpha = 0;
    double dctcp_gamma = 0.0625; // 1/16 (backup 0.16) TODO make it NED parameter;
};

cplusplus(DcTcpFamilyStateVariables) {{
  public:
    virtual std::string str() const override;
    virtual std::string detailedInfo() const override;
}}

