//
// Copyright (C) 2008 Juan-Carlos Maureira
// Copyright (C) INRIA
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
#include "MACAddress.h"
#include "IPv4Address.h"
#include "DHCPOptions.h"
}}


class noncobject MACAddress;
class noncobject IPv4Address;
class noncobject DHCPOption;

enum DHCP_OPCODE
{
    BOOTREQUEST = 1;
    BOOTREPLY = 2;
}

enum DHCP_TYPE
{
    DHCPDISCOVER = 1;
    DHCPOFFER = 2;
    DHCPREQUEST = 3;
    DHCPDECLINE = 4;
    DHCPACK = 5;
    DHCPNAK = 6;
    DHCPRELEASE = 7;
    DHCPINFORM = 8;
    DHCPFORCERENEW = 9;
    DHCPLEASEQUERY = 10;
    DHCPLEASEUNASSIGNED = 11;
    DHCPLEASEUNKNOWN = 12;
    DHCPLEASEACTIVE = 13;
}

//
// DHCP Messages
//
// To be used by the DHCP client and server
//
packet DHCPMessage
{
    int op;
    int htype;
    int hlen;
    int hops;
    long xid;
    int secs;
    long flags;
    IPv4Address ciaddr;
    IPv4Address yiaddr;
    IPv4Address giaddr;
    MACAddress chaddr;
    string sname;
    string file;
    DHCPOption options;
}

