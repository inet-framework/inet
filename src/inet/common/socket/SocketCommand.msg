//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

