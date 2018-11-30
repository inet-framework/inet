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
import inet.common.Protocol;
import inet.linklayer.common.MacAddress;

namespace inet;

enum EthernetCommandCode
{
    ETHERNET_C_DATA = 0;
    ETHERNET_C_BIND = 1;
    ETHERNET_C_CLOSE = 2;
    ETHERNET_C_DESTROY = 3;
}

enum EthernetStatusInd
{
    ETHERNET_I_DATA = 0;
    ETHERNET_I_SOCKET_CLOSED = 2;
}

class EthernetCommandBase extends cObject
{
}

class EthernetBindCommand extends EthernetCommandBase
{
    MacAddress sourceAddress;
    MacAddress destinationAddress;
    int vlanId;
	Protocol *protocol;
}

class EthernetCloseCommand extends EthernetCommandBase
{
}

class EthernetDestroyCommand extends EthernetCommandBase
{
}

class EthernetSocketClosedIndication extends EthernetCommandBase
{
}

