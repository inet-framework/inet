//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp.flavours;

//
// Reno (RFC 5681), with the SACK recovery of RFC 6675 when SACK is on.
//
simple TcpReno extends TcpAlgorithm
{
    parameters:
        @class(TcpReno);
}
