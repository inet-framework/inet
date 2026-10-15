//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp.flavours;

//
// Westwood: the window after a loss follows an estimate of the bandwidth.
//
simple TcpWestwood extends TcpAlgorithm
{
    parameters:
        @class(TcpWestwood);
}
