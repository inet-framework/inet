//
// Copyright (C) 2006 Alfonso Ariza Quintana
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

class noncobject IPAddress;

cplusplus {{
#include "IPAddress.h"
}}



enum ManetControlType
{

    MANET_ROUTE_NO_OPTION = 0;
    MANET_ROUTE_UPDATE = 1;
    MANET_ROUTE_NOROUTE = 2;
}

//
// ICMP message class
//
packet ControlManetRouting
{
    IPAddress srcAddress;
    IPAddress destAddress;
    int optionCode enum(ManetControlType) = MANET_ROUTE_NO_OPTION;
}
