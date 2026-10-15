//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp.flavours;

//
// Vegas: the window follows the difference of the expected and the actual
// rate.
//
simple TcpVegas extends TcpAlgorithm
{
    parameters:
        @class(TcpVegas);
}
