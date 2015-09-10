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
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
}}

namespace inet;

class noncobject MACAddress;
class noncobject IPv4Address;

enum DHCPOpcode
{
    BOOTREQUEST = 1;
    BOOTREPLY = 2;
}

enum DHCPMessageType
{
    DHCPDISCOVER = 1;
    DHCPOFFER = 2;
    DHCPREQUEST = 3;
    DHCPDECLINE = 4;
    DHCPACK = 5;
    DHCPNAK = 6;
    DHCPRELEASE = 7;
    DHCPINFORM = 8;
}

enum DHCPOptionCode
{
    DHCP_MSG_TYPE = 53;
    CLIENT_ID = 61;
    HOSTNAME = 12;
    REQUESTED_IP = 50;
    PARAM_LIST = 55;
    SUBNET_MASK = 1;
    ROUTER = 3;
    DNS = 6;
    NTP_SRV = 42;
    RENEWAL_TIME = 58;
    REBIND_TIME = 59;
    LEASE_TIME = 51;
    SERVER_ID = 54;
}

//
// Represents options in a DHCP message. In the DHCP protocol, options are
// added to the message as tagged data items. In the simulation, this DHCPOptions
// class statically holds the union of all options actually used by the DHCP
// protocol models. Options absent from a packet are represented by empty/unfilled
// DHCPOptions fields.
//
// @see DHCPMessage, DHCPClient, DHCPServer
//
class DHCPOptions {
    int messageType @enum(DHCPMessageType); // to convey the type of the DHCP message
    string hostName; // name of the client
    int parameterRequestList[]; // used by a DHCP client to request values for specified configuration parameters
    MACAddress clientIdentifier; // used by DHCP clients to specify their unique identifier
    IPv4Address requestedIp; // used in a client request (DHCPDISCOVER) to allow the client to request that a particular IP address be assigned
    IPv4Address subnetMask; // client's subnet mask
    IPv4Address router[]; // IP addresses for routers on the client's subnet
    IPv4Address dns[]; // list of DNSs available to the client
    IPv4Address ntp[]; // list of IP addresses indicating NTP servers available to the client
    IPv4Address serverIdentifier; // client use this field as the destination address for any unicast DHCP messages to the server
    simtime_t renewalTime; // time interval (T1) from address assignment until the client transitions to the RENEWING state
    simtime_t rebindingTime; // time interval (T2) from address assignment until the client transitions to the REBINDING state
    simtime_t leaseTime; // request for lease time (client), offered lease time (server)
}

//
// Represents a DHCP message. DHCP (Dynamic Host Configuration Protocol, RFC 2131)
// provides a framework for passing configuration information to hosts on a TCP/IP network.
//
// @see DHCPClient, DHCPServer
//
packet DHCPMessage
{
    int op @enum(DHCPOpcode); // message op code / message type. 1 = BOOTREQUEST, 2 = BOOTREPLY
    int htype; // hardware address type, see ARP section in "Assigned Numbers" RFC; e.g., '1' = 10mb ethernet
    int hlen; // hardware address length (e.g.  '6' for 10mb ethernet)
    int hops; // client sets to zero, optionally used by relay agents when booting via a relay agent
    unsigned int xid; // transaction ID, a random number chosen by the client, used by the client and server to associate messages and responses between a client and a server
    int secs; // filled in by client, seconds elapsed since client began address acquisition or renewal process
    bool broadcast; // the broadcast bit in the flags field
    IPv4Address ciaddr; // client IP address; only filled in if client is in BOUND, RENEW or REBINDING state and can respond to ARP requests
    IPv4Address yiaddr; // 'your' (client) IP address
    IPv4Address giaddr; // relay agent IP address, used in booting via a relay agent
    MACAddress chaddr; // client hardware address (MAC)
    string sname; // optional server host name
    string file; // boot file name (unused in the simulation)
    DHCPOptions options; // holds DHCP options
}

