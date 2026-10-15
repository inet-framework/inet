//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp.flavours;

import inet.common.SimpleModule;

//
// The congestion control and loss recovery algorithm of one TCP connection.
// Each connection of ~Tcp creates one as its submodule `algorithm`, of the NED
// type that the `tcpAlgorithmClass` parameter of ~Tcp names, or that the socket
// chose. The connection calls the algorithm from C++, and the algorithm extends
// the state of the connection with its own variables; Qtenv shows that state
// with the parameters of the algorithm.
//
simple TcpAlgorithm extends SimpleModule
{
    parameters:
        @class(-);
        @display("i=block/control");
}
