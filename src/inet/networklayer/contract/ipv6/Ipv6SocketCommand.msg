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

