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
/// State variables for DumbTcp.
///
struct DumbTcpStateVariables extends TcpStateVariables
{
    @descriptor(readonly);
};

