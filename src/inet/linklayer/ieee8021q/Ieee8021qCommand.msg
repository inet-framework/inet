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

namespace inet;

enum Ieee8021qCommandCode
{
    IEEE8021Q_C_DATA = 0;
    IEEE8021Q_C_BIND = 1;
    IEEE8021Q_C_CLOSE = 2;
    IEEE8021Q_C_DESTROY = 3;
}

enum Ieee8021qStatusInd
{
    IEEE8021Q_I_DATA = 0;
    IEEE8021Q_I_SOCKET_CLOSED = 2;
}

class Ieee8021qCommandBase extends cObject
{
}

class Ieee8021qBindCommand extends Ieee8021qCommandBase
{
    const Protocol *protocol;
    int vlanId;
    bool steal;
}

class Ieee8021qCloseCommand extends Ieee8021qCommandBase
{
}

class Ieee8021qDestroyCommand extends Ieee8021qCommandBase
{
}

class Ieee8021qSocketClosedIndication extends Ieee8021qCommandBase
{
}

