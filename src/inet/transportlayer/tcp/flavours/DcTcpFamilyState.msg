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
    uint32_t dctcp_deliveredCeBytesMark = 0; // AccECN (Workstream G8): snapshot of deliveredCeBytes as of the last receivedDataAck() call, for computing this round's byte-exact CE increment (same pattern as prrDeliveredMark for deliveredBytes)
    uint32_t dctcp_deliveredCePktsMark = 0; // AccECN (Workstream G8): snapshot of deliveredCePkts, for the ACE-only (no AccECN option) fallback estimate when the byte-exact one above didn't advance this round
    double dctcp_alpha = 0;
    double dctcp_gamma = 0.0625; // 1/16 (backup 0.16) TODO make it NED parameter;
};

cplusplus(DcTcpFamilyStateVariables) {{
  public:
    virtual std::string str() const override;
    virtual std::string detailedInfo() const override;
}}

