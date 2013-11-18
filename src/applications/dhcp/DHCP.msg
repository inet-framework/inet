//
// Copyright (C) 2008 Juan-Carlos Maureira
// Copyright (C) INRIA
// Copyright (C) 2013 OpenSim Ltd.
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

// For more information, see RFC 2132: 'DHCP Options and BOOTP Vendor Extensions'
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
// DHCP message packet
// Implemented according to the RFC 2131: Figure 1 'Format of a DHCP message'
//
packet DHCPMessage
{
    int op; // message op code / message type. 1 = BOOTREQUEST, 2 = BOOTREPLY
    int htype; // hardware address type, see ARP section in "Assigned Numbers" RFC; e.g., '1' = 10mb ethernet
    int hlen; // hardware address length (e.g.  '6' for 10mb ethernet)
    int hops; // client sets to zero, optionally used by relay agents when booting via a relay agent
    long xid; // transaction ID, a random number chosen by the client, used by the client and server to associate messages and responses between a client and a server
    int secs; // filled in by client, seconds elapsed since client began address acquisition or renewal process
    long flags; // broadcast flag
    IPv4Address ciaddr; // client IP address; only filled in if client is in BOUND, RENEW or REBINDING state and can respond to ARP requests
    IPv4Address yiaddr; // 'your' (client) IP address
    IPv4Address giaddr; // relay agent IP address, used in booting via a relay agent
    MACAddress chaddr; // client hardware address (MAC)
    string sname; // optional server host name
    string file; // boot file name, null terminated string; "generic" name or null in DHCPDISCOVER, fully qualified directory-path name in DHCPOFFER.
    DHCPOption options; // optional parameters field. for more information see DHCPOption class.
}

