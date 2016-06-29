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

cplusplus {{
#include "inet/common/Protocol.h"
#include "inet/common/ISocketControlInfo.h"
}}

namespace inet;

class L3SocketCommand
{
    @implements(ISocketControlInfo);
    int socketId;
}

class L3SocketBindCommand extends L3SocketCommand
{
    int protocolId;
}

class L3SocketCloseCommand extends L3SocketCommand
{
}

