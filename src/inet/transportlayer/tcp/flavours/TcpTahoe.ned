//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp.flavours;

//
// Tahoe: slow start, congestion avoidance and fast retransmit, without fast
// recovery.
//
simple TcpTahoe extends TcpAlgorithm
{
    parameters:
        @class(TcpTahoe);
}
