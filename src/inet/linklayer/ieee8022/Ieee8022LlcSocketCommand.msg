//
// Copyright (C) 2018 OpenSim Ltd.
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

enum Ieee8022LlcSocketCommandCode
{
    IEEE8022_LLC_C_OPEN  = 1;
    IEEE8022_LLC_C_CLOSE = 2;
    IEEE8022_LLC_C_DESTROY = 3;
    IEEE8022_LLC_C_DATA = 4;
}

enum Ieee8022LlcSocketIndicationCode
{
    IEEE8022_LLC_I_SOCKET_CLOSED = 2;
    IEEE8022_LLC_I_DATA = 4;
}

class Ieee8022LlcSocketCommand extends cObject
{
}

class Ieee8022LlcSocketOpenCommand extends Ieee8022LlcSocketCommand
{
    int localSap;
}

class Ieee8022LlcSocketCloseCommand extends Ieee8022LlcSocketCommand
{
}

class Ieee8022LlcSocketDestroyCommand extends Ieee8022LlcSocketCommand
{
}

class Ieee8022LlcSocketClosedIndication extends Ieee8022LlcSocketCommand
{
}

