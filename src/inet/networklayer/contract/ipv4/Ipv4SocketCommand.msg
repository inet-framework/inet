//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Protocol;
import inet.networklayer.contract.ipv4.Ipv4Address;

namespace inet;

enum Ipv4CommandCode
{
    IPv4_C_BIND    = 1;
    IPv4_C_CONNECT = 2;
    IPv4_C_CLOSE   = 3;
    IPv4_C_DESTROY = 4;
}

enum Ipv4StatusInd {
    IPv4_I_DATA = 0;
    IPv4_I_SOCKET_CLOSED = 1;
};

class Ipv4SocketCommandBase extends cObject
{
}

class Ipv4SocketBindCommand extends Ipv4SocketCommandBase
{
    const Protocol *protocol;
    Ipv4Address localAddress;
}

class Ipv4SocketConnectCommand extends Ipv4SocketCommandBase
{
    Ipv4Address remoteAddress;
}

class Ipv4SocketCloseCommand extends Ipv4SocketCommandBase
{
}

class Ipv4SocketDestroyCommand extends Ipv4SocketCommandBase
{
}

class Ipv4SocketClosedIndication extends Ipv4SocketCommandBase
{
}
