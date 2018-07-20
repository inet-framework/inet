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
import inet.networklayer.common.IpProtocolId;   //FIXME remove it, unused
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
    UDP_I_ERROR = 1; // ~Udp attaches ~UDPIndication to received data packet
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
//
// @see ~UdpCommandCode
//
class UdpCloseCommand extends UdpControlInfo
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
    int ttl;
}

//
// Control info for setting the Type of Service (Ipv4) or Traffic Class (Ipv6)
// header field on outgoing Ipv4/Ipv6 packets sent from an Udp socket.
// This is an 8-bit field, the lowest 6 bits are the Dscp (DiffServ Code Point)
// and the highest 2 bits are the ECN (Explicit Congestion Notification).
// To set the option, send a message to  the ~Udp module with kind=UDP_C_SETOPTION
// and an and instance of this control info class attached.
//
// @see ~UdpCommandCode
//
class UdpSetTypeOfServiceCommand extends UdpSetOptionCommand
{
    char tos;  // ToS byte for Ipv4/Ipv6
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
    L3Address multicastAddr[];
}

//
// Control info for letting an UDP socket to block multicast traffic from
// specific sources after joining a multicast group.
//
// see IP_BLOCK_SOURCE socket option
class UdpBlockMulticastSourcesCommand extends UdpSetOptionCommand
{
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
    int interfaceId;
    L3Address multicastAddr;
    UdpSourceFilterMode filterMode;
    L3Address sourceList[];
}

