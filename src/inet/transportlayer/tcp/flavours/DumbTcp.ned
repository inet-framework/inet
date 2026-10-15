//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp.flavours;

//
// A very basic algorithm: a fixed retransmission timeout, no delayed ACKs, no
// silly window avoidance and no congestion control. For tests and teaching.
//
simple DumbTcp extends TcpAlgorithm
{
    parameters:
        @class(DumbTcp);
}
