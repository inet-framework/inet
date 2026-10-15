//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp.flavours;

//
// NewReno (RFC 6582), with the SACK recovery of RFC 6675 when SACK is on.
//
simple TcpNewReno extends TcpAlgorithm
{
    parameters:
        @class(TcpNewReno);
}
