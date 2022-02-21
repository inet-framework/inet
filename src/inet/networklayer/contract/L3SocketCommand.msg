//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Protocol;
import inet.networklayer.common.L3Address;

namespace inet;

enum L3CommandCode
{
    L3_C_BIND    = 1;
    L3_C_CONNECT = 2;
    L3_C_CLOSE   = 3;
    L3_C_DESTROY = 4;
}

enum L3StatusInd {
    L3_I_DATA = 0;
    L3_I_SOCKET_CLOSED = 1;
}

class L3SocketCommandBase extends cObject
{
}

class L3SocketBindCommand extends L3SocketCommandBase
{
    const Protocol *protocol;
    L3Address localAddress;
}

class L3SocketConnectCommand extends L3SocketCommandBase
{
    L3Address remoteAddress;
}

class L3SocketCloseCommand extends L3SocketCommandBase
{
}

class L3SocketDestroyCommand extends L3SocketCommandBase
{
}

class L3SocketClosedIndication extends L3SocketCommandBase
{
}
