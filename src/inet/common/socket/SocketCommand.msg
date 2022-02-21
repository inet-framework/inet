//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Protocol;
import inet.linklayer.common.MacAddress;

namespace inet;

enum SocketCommandCode
{
    SOCKET_C_DATA = 0;
    SOCKET_C_BIND = 1;
    SOCKET_C_OPEN = 2;
    SOCKET_C_CLOSE = 3;
    SOCKET_C_DESTROY = 4;
}

enum SocketStatusInd
{
    SOCKET_I_DATA = 0;
    SOCKET_I_CLOSED = 1;
}

class SocketCommandBase extends cObject
{
}

class SocketBindCommand extends SocketCommandBase
{
    bool steal;
}

class SocketCloseCommand extends SocketCommandBase
{
}

class SocketDestroyCommand extends SocketCommandBase
{
}

class SocketClosedIndication extends SocketCommandBase
{
}
