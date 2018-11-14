//
// Copyright (C) 2015 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
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

