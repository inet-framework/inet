//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.networklayer.common.L3Address;

namespace inet;

//
// Udp command codes, sent by the application to ~Udp. These constants
// should be set as message kind on messages sent to the ~Udp entity.
//
// @see ~UdpControlInfo, ~UdpStatusInd, ~Udp
//
enum UdpCommandCode
{
    UDP_C_DATA = 0;      // send()/sendTo(); see ~UDPSendCommand
    UDP_C_BIND = 1;      // bind(); see ~UdpBindCommand
    UDP_C_CONNECT = 2;   // connect(); see ~UdpConnectCommand
    UDP_C_SETOPTION = 3; // setsockopt(); see ~UdpSetTimeToLiveCommand,...
    UDP_C_CLOSE = 4;     // close(); see ~UdpCloseCommand
    UDP_C_DESTROY = 5;   // destroy(); see ~UdpDestroyCommand
}

enum UdpSetOptionSubcode
{
    UDP_C_SETOPTION_TTL = 1;
    UDP_C_SETOPTION_DSCP = 2;
    UDP_C_SETOPTION_TOS = 3;
    UDP_C_SETOPTION_BROADCAST = 4;
    UDP_C_SETOPTION_MCAST_IFACE = 5;
    UDP_C_SETOPTION_MCAST_LOOP = 6;
    UDP_C_SETOPTION_REUSEADDR = 7;
    UDP_C_SETOPTION_JOIN_MCAST_GRP = 8;
    UDP_C_SETOPTION_LEAVE_MCAST_GRP = 9;
    UDP_C_SETOPTION_JOIN_MCAST_SRC = 10;
    UDP_C_SETOPTION_BLOCK_MCAST_SRC = 11;
    UDP_C_SETOPTION_UNBLOCK_MCAST_SRC = 12;
    UDP_C_SETOPTION_LEAVE_MCAST_SRC = 13;
    UDP_C_SETOPTION_SET_MCAST_SRC_FILTER = 14;
}

//
// Udp indications, sent by ~Udp to the application. ~Udp will set these
// constants as message kind on messages it sends to the application.
//
// @see ~UdpControlInfo, ~UdpCommandCode, ~Udp
//
enum UdpStatusInd
{
    UDP_I_DATA = 0;  // ~Udp attaches ~UDPIndication to received data packet
    UDP_I_ERROR = 1;
    UDP_I_SOCKET_CLOSED = 2;
}

//
// Base class for UDP control info classes. The most convenient way to handle
// ~Udp is the UdpSocket class, which hides control info from you.
//
// @see ~UdpCommandCode
//
class UdpControlInfo extends cObject
{
}

//
// Control info for binding an UDP socket. To create and bind a socket,
// send a message to the ~Udp module with kind=UDP_C_BIND and an
// ~UdpBindCommand attached.
//
// Both the address and the port may be left unset. If the port is
// unset, ~Udp will assign an ephemeral port.
//
// @see ~UdpCommandCode
//
class UdpBindCommand extends UdpControlInfo
{
    L3Address localAddr;
    int localPort = -1;
}

//
// Control info for connecting an UDP socket. To connect a socket,
// send a message to the ~Udp module with kind=UDP_C_CONNECT and an
// ~UdpConnectCommand attached. If the socket does not exist,
// it will be created.
//
// Both the address and the port must be filled in.
//
// @see ~UdpCommandCode
//
class UdpConnectCommand extends UdpControlInfo
{
    L3Address remoteAddr;
    int remotePort = -1;
}

//
// Control info for sending data via UDP. To send a packet, send it to
// the ~Udp module with kind=UDP_C_SEND and an ~UDPSendCommand attached.
//
// UDP_C_SEND/~UDPSendCommand models both the Unix send() and sendto() calls.
// If the socket is connected, tags not required, or destAddr/destPort may be left blank (send()),
// otherwise it must contain the destination for the packet (sendto()).
//
// @see ~UdpCommandCode, ~PortsReq, ~L3AddressReq, ~InterfaceReq
//

//
// Control info for closing an UDP socket. To close a socket, send a message
// to the ~Udp module with kind=UDP_C_CLOSE and an ~UdpCloseCommand attached.
// The Udp module will send an ~UdpSocketClosedIndication answer.
//
// @see ~UdpCommandCode
//
class UdpCloseCommand extends UdpControlInfo
{
}

//
// Control info for inform app about an UDP socket closed.
// This is answer to an ~UdpCloseCommand.
//
// @see ~UdpCommandCode
//
class UdpSocketClosedIndication extends UdpControlInfo
{
}

//
// Control info for destroying an UDP socket. To destroy a socket, send a message
// to the ~Udp module with kind=UDP_C_DESTROY and an ~UdpDestroyCommand attached.
//
// @see ~UdpCommandCode
//
class UdpDestroyCommand extends UdpControlInfo
{
}

//
// Control info that is attached to received data packets, sent up from the
// ~Udp module to the application with UDP_I_DATA as message kind.
//
// @see ~UdpCommandCode, ~L3AddressInd, ~PortsInd
//

//
// Control info that is sent up from the ~Udp module to the application with
// UDP_I_ERROR as message kind when ~Udp receives an Icmp error for a packet
// previously sent from the socket.
//
// @see ~UdpCommandCode
//
class UdpErrorIndication extends UdpControlInfo
{
}

//
// Base class for UDP socket option control info classes.
//
class UdpSetOptionCommand extends UdpControlInfo
{
    UdpSetOptionSubcode optionCode = static_cast<UdpSetOptionSubcode>(-1);
}

//
// Control info for setting the Time To Live (a.k.a. Hop Limit) option on an
// UDP socket. This option will affect both multicast and unicast packets.
// To set the option, send a message to the ~Udp module with kind=UDP_C_SETOPTION
// and an and instance of this control info class attached.
//
// @see ~UdpCommandCode
//
class UdpSetTimeToLiveCommand extends UdpSetOptionCommand
{
    optionCode = UDP_C_SETOPTION_TTL;
    int ttl;
}

//
// Control info for setting the DSCP (DiffServ Code Point)
// header field on outgoing Ipv4/Ipv6 packets sent from an Udp socket.
// This is a 6-bit field.
// To set the option, send a message to  the ~Udp module with kind=UDP_C_SETOPTION
// and an and instance of this control info class attached.
//
// @see ~UdpCommandCode
//
class UdpSetDscpCommand extends UdpSetOptionCommand
{
    optionCode = UDP_C_SETOPTION_DSCP;
    short dscp;  // diffserv code point for Ipv4/Ipv6
}

//
// Control info for setting the TOS (Type Of Service) / Traffic Class
// header field on outgoing Ipv4/Ipv6 packets sent from an Udp socket.
// This is a 8-bit field.
// To set the option, send a message to  the ~Udp module with kind=UDP_C_SETOPTION
// and an and instance of this control info class attached.
//
// @see ~UdpCommandCode
//
class UdpSetTosCommand extends UdpSetOptionCommand
{
    optionCode = UDP_C_SETOPTION_TOS;
    short tos;  // type of service for Ipv4 / Traffic class for Ipv6
}

//
// Control info for setting the Broadcast option on an UDP socket.
// To set the option, send a message to the ~Udp module with kind=UDP_C_SETOPTION
// and an and instance of this control info class attached.
//
// @see ~UdpCommandCode
//
class UdpSetBroadcastCommand extends UdpSetOptionCommand
{
    optionCode = UDP_C_SETOPTION_BROADCAST;
    bool broadcast;
}

//
// Control info for setting the multicast output interface for an UDP socket.
// To set the option, send a message to the ~Udp module with kind=UDP_C_SETOPTION
// and an and instance of this control info class attached.
//
// @see ~UdpCommandCode
//
class UdpSetMulticastInterfaceCommand extends UdpSetOptionCommand
{
    optionCode = UDP_C_SETOPTION_MCAST_IFACE;
    int interfaceId;
}

//
// Control info for setting the multicast loop option for an UDP socket.
// When the option is true, a copy of the outgoing multicast packet
// is delivered locally via the loopback interface.
// To set the option, send a message to the ~Udp module with kind=UDP_C_SETOPTION
// and an instance of this control info class attached.
//
// @see ~UdpCommandCode
//
class UdpSetMulticastLoopCommand extends UdpSetOptionCommand
{
    optionCode = UDP_C_SETOPTION_MCAST_LOOP;
    bool loop;
}

//
// Control info for setting the ReuseAddress option on an UDP socket.
// When the option is true, then the socket is allowed to bind to an already bound
// local address.
// To set the option, send a message to the ~Udp module with kind=UDP_C_SETOPTION
// and an and instance of this control info class attached.
//
// @see ~UdpCommandCode
//
class UdpSetReuseAddressCommand extends UdpSetOptionCommand
{
    optionCode = UDP_C_SETOPTION_REUSEADDR;
    bool reuseAddress;
}

//
// Control info for letting an UDP socket join multicast groups.
// To set the option, send a message to the ~Udp module with kind=UDP_C_SETOPTION
// and an and instance of this control info class attached.
//
// The address-interfaceId pairs are passed as two separate arrays.
// When there are less interfaceIds than multicast addresses,
// then -1 is assumed (meaning join on all interfaces).
//
// @see ~UdpCommandCode
//
class UdpJoinMulticastGroupsCommand extends UdpSetOptionCommand
{
    optionCode = UDP_C_SETOPTION_JOIN_MCAST_GRP;
    L3Address multicastAddr[];
    int interfaceId[];
}

//
// Control info for letting an UDP socket leave multicast groups.
// To set the option, send a message to the ~Udp module with kind=UDP_C_SETOPTION
// and an and instance of this control info class attached.
//
// @see ~UdpCommandCode
//
class UdpLeaveMulticastGroupsCommand extends UdpSetOptionCommand
{
    optionCode = UDP_C_SETOPTION_LEAVE_MCAST_GRP;
    L3Address multicastAddr[];
}

//
// Control info for letting an UDP socket to block multicast traffic from
// specific sources after joining a multicast group.
//
// see IP_BLOCK_SOURCE socket option
class UdpBlockMulticastSourcesCommand extends UdpSetOptionCommand
{
    optionCode = UDP_C_SETOPTION_BLOCK_MCAST_SRC;
    int interfaceId;
    L3Address multicastAddr;
    L3Address sourceList[];
}

//
// Control info for letting an UDP socket to permit multicast traffic from
// a previously blocked source.
//
// see IP_UNBLOCK_SOURCE socket option
class UdpUnblockMulticastSourcesCommand extends UdpSetOptionCommand
{
    optionCode = UDP_C_SETOPTION_UNBLOCK_MCAST_SRC;
    int interfaceId;
    L3Address multicastAddr;
    L3Address sourceList[];
}

//
// Control info for letting an UDP socket to join a multicast group
// only for the selected sources.
//
// see IP_ADD_SOURCE_MEMBERSHIP socket option
class UdpJoinMulticastSourcesCommand extends UdpSetOptionCommand
{
    optionCode = UDP_C_SETOPTION_JOIN_MCAST_SRC;
    int interfaceId;
    L3Address multicastAddr;
    L3Address sourceList[];
}

//
// Control info for letting an UDP socket to leave a multicast group
// for the selected sources.
//
// see IP_DROP_SOURCE_MEMBERSHIP socket option
class UdpLeaveMulticastSourcesCommand extends UdpSetOptionCommand
{
    optionCode = UDP_C_SETOPTION_LEAVE_MCAST_SRC;
    int interfaceId;
    L3Address multicastAddr;
    L3Address sourceList[];
}

enum UdpSourceFilterMode { UDP_INCLUDE_MCAST_SOURCES = 1; UDP_EXCLUDE_MCAST_SOURCES = 2; }

//
// Control info for letting an UDP socket to specify the excluded/included
// sources for a multicast group.
//
// Unlike Block/Unblock/Join/LeaveMulticastSource commands, this method is
// not incremental.
//
class UdpSetMulticastSourceFilterCommand extends UdpSetOptionCommand
{
    optionCode = UDP_C_SETOPTION_SET_MCAST_SRC_FILTER;
    int interfaceId;
    L3Address multicastAddr;
    UdpSourceFilterMode filterMode;
    L3Address sourceList[];
}
