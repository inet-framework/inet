//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Protocol;
import inet.networklayer.contract.ipv6.Ipv6Address;

namespace inet;

enum Ipv6CommandCode
{
    IPv6_C_BIND    = 1;
    IPv6_C_CONNECT = 2;
    IPv6_C_CLOSE   = 3;
    IPv6_C_DESTROY = 4;
}

enum Ipv6StatusInd {
    IPv6_I_DATA = 0;
    IPv6_I_SOCKET_CLOSED = 1;
};

class Ipv6SocketCommandBase extends cObject
{
}

class Ipv6SocketBindCommand extends Ipv6SocketCommandBase
{
    const Protocol *protocol;
    Ipv6Address localAddress;
}

class Ipv6SocketConnectCommand extends Ipv6SocketCommandBase
{
    const Protocol *protocol;
    Ipv6Address remoteAddress;
}

class Ipv6SocketCloseCommand extends Ipv6SocketCommandBase
{
}

class Ipv6SocketDestroyCommand extends Ipv6SocketCommandBase
{
}

class Ipv6SocketClosedIndication extends Ipv6SocketCommandBase
{
}
