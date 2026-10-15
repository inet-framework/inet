//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp.flavours;

//
// CUBIC (RFC 9438), as Linux tcp_cubic.c, with its slow start (HyStart).
//
simple TcpCubic extends TcpAlgorithm
{
    parameters:
        @class(TcpCubic);
}
