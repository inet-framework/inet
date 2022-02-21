//
// Copyright (C) 2008 Juan-Carlos Maureira
// Copyright (C) INRIA
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.linklayer.common.MacAddress;

namespace inet;

enum DhcpOpcode
{
    BOOTREQUEST = 1;
    BOOTREPLY = 2;
}

enum DhcpMessageType
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

enum DhcpOptionCode
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

// DHCP timer types (RFC 2131 4.4.5)
enum DhcpTimerType
{
    WAIT_OFFER = 1;
    WAIT_ACK = 2;
    T1 = 3;
    T2 = 4;
    LEASE_TIMEOUT = 5;
    START_DHCP = 6;
}

//
// Represents options in a DHCP message. In the DHCP protocol, options are
// added to the message as tagged data items. In the simulation, this DhcpOptions
// class statically holds the union of all options actually used by the DHCP
// protocol models. Options absent from a packet are represented by empty/unfilled
// DhcpOptions fields.
//
// @see DhcpMessage, DhcpClient, DhcpServer
//
class DhcpOptions extends cObject
{
    @packetData;
    DhcpMessageType messageType; // to convey the type of the DHCP message
    string hostName; // name of the client
    DhcpOptionCode parameterRequestList[]; // used by a DHCP client to request values for specified configuration parameters
    MacAddress clientIdentifier; // used by DHCP clients to specify their unique identifier
    Ipv4Address requestedIp; // used in a client request (DHCPDISCOVER) to allow the client to request that a particular IP address be assigned
    Ipv4Address subnetMask; // client's subnet mask
    Ipv4Address router[]; // IP addresses for routers on the client's subnet
    Ipv4Address dns[]; // list of DNSs available to the client
    Ipv4Address ntp[]; // list of IP addresses indicating NTP servers available to the client
    Ipv4Address serverIdentifier; // client use this field as the destination address for any unicast DHCP messages to the server
    simtime_t renewalTime; // time interval (T1) from address assignment until the client transitions to the RENEWING state
    simtime_t rebindingTime; // time interval (T2) from address assignment until the client transitions to the REBINDING state
    simtime_t leaseTime; // request for lease time (client), offered lease time (server)
}

//
// Represents a DHCP message. DHCP (Dynamic Host Configuration Protocol, RFC 2131)
// provides a framework for passing configuration information to hosts on a TCP/IP network.
//
// @see DhcpClient, DhcpServer
//
class DhcpMessage extends FieldsChunk
{
    DhcpOpcode op; // message op code / message type. 1 = BOOTREQUEST, 2 = BOOTREPLY
    int htype; // hardware address type, see Arp section in "Assigned Numbers" RFC; e.g., '1' = 10mb ethernet
    int hlen; // hardware address length (e.g.  '6' for 10mb ethernet)
    int hops; // client sets to zero, optionally used by relay agents when booting via a relay agent
    unsigned int xid; // transaction ID, a random number chosen by the client, used by the client and server to associate messages and responses between a client and a server
    int secs; // filled in by client, seconds elapsed since client began address acquisition or renewal process
    bool broadcast; // the broadcast bit in the flags field
    uint16_t reserved = 0; // MUST BE ZERO (reserved for future use)
    Ipv4Address ciaddr; // client IP address; only filled in if client is in BOUND, RENEW or REBINDING state and can respond to Arp requests
    Ipv4Address yiaddr; // 'your' (client) IP address
    Ipv4Address giaddr; // relay agent IP address, used in booting via a relay agent
    MacAddress chaddr; // client hardware address (MAC)
    string sname; // optional server host name
    string file; // boot file name (unused in the simulation)
    DhcpOptions options; // holds DHCP options
}
