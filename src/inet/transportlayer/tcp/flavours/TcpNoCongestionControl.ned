//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp.flavours;

//
// TCP without congestion control: the congestion window stays very large.
// For tests and teaching.
//
simple TcpNoCongestionControl extends TcpAlgorithm
{
    parameters:
        @class(TcpNoCongestionControl);
}
