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
    uint32_t dctcp_deliveredCePktsMark = 0; // AccECN (Workstream G8): snapshot of deliveredCePkts, for the ACE-only (no AccECN option ever seen yet) fallback estimate
    bool dctcp_accEcnOptionSeen = false; // AccECN (Workstream G8): latches true the first time this connection ever sees a real AccECN option; selects the byte-exact lens EXCLUSIVELY from then on (never per-round -- see receivedDataAck()'s comment for why mixing lenses per round double-counts)
    double dctcp_alpha = 0;
    double dctcp_gamma = 0.0625; // 1/16 (backup 0.16) TODO make it NED parameter;
};

cplusplus(DcTcpFamilyStateVariables) {{
  public:
    virtual std::string str() const override;
    virtual std::string detailedInfo() const override;
}}

