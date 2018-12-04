//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 Andras Varga
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

namespace inet;

enum TunCommandCode
{
    TUN_C_OPEN  = 1;
    TUN_C_CLOSE = 2;
    TUN_C_DESTROY = 3;
    TUN_C_DATA = 4;
}

enum TunSocketIndication
{
    TUN_I_CLOSED = 2;
    TUN_I_DATA = 4;
}

class TunControlInfo extends cObject
{
}

class TunOpenCommand extends TunControlInfo
{
}

class TunCloseCommand extends TunControlInfo
{
}

class TunDestroyCommand extends TunControlInfo
{
}

class TunSocketClosedIndication extends TunControlInfo
{
}

