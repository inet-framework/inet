//
// Copyright (C) 2013 OpenSim Ltd.
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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

cplusplus {{
    #include "MACAddress.h"
    #include "IPv4Address.h"
}}

class noncobject MACAddress;
class noncobject IPv4Address;

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
    DHCPFORCERENEW = 9;
    DHCPLEASEQUERY = 10;
    DHCPLEASEUNASSIGNED = 11;
    DHCPLEASEUNKNOWN = 12;
    DHCPLEASEACTIVE = 13;
}

enum OpCode
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
// The Dynamic Host Configuration Protocol (DHCP) provides a framework for
// passing configuration information to hosts on a TCP/IP network.
// Configuration parameters and other control information are carried in
// tagged data items that are stored in the 'options' field of the DHCP message.
//
// This packet contains some selected options for a DHCP message. These
// fields are sufficient for DHCP to work properly.
//
// For more information, see RFC 2132.
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
