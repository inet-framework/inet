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

