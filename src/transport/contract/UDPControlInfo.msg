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


cplusplus {{
#include "IPvXAddress.h"
}}

class noncobject IPvXAddress;


//
// UDP command codes, sent by the application to ~UDP. These constants
// should be set as message kind on messages sent to the ~UDP entity.
//
// @see ~UDPControlInfo, ~UDPStatusInd, ~UDP
//
enum UDPCommandCode
{
    UDP_C_DATA = 0;      // send()/sendTo(); see ~UDPSendCommand
    UDP_C_BIND = 1;      // bind(); see ~UDPBindCommand
    UDP_C_CONNECT = 2;   // connect(); see ~UDPConnectCommand
    UDP_C_SETOPTION = 3; // setsockopt(); see ~UDPSetTimeToLiveCommand,...
    UDP_C_CLOSE = 4;     // close(); see ~UDPCloseCommand
}


//
// UDP indications, sent by ~UDP to the application. ~UDP will set these
// constants as message kind on messages it sends to the application.
//
// @see ~UDPControlInfo, ~UDPCommandCode, ~UDP
//
enum UDPStatusInd
{
    UDP_I_DATA = 0;  // ~UDP attaches ~UDPIndication to received data packet
    UDP_I_ERROR = 1; // ~UDP attaches ~UDPIndication to received data packet
}


//
// Base class for UDP control info classes. The most convenient way to handle
// ~UDP is the UDPSocket class, which hides control info from you.
//
// sockId must uniquely identify the socket in the ~UDP module; the best way
// to achieve that is to obtain the value from UDPSocket::generateSocketId().
//
// @see ~UDPCommandCode
//
class UDPControlInfo
{
    int sockId = -1;  // must uniquely identify the socket in the ~UDP module
}

//
// Control info for binding an UDP socket. To create and bind a socket,
// send a message to the ~UDP module with kind=UDP_C_BIND and an
// ~UDPBindCommand attached.
//
// Both the address and the port may be left unset. If the port is
// unset, ~UDP will assign an ephemeral port.
//
// @see ~UDPCommandCode
//
class UDPBindCommand extends UDPControlInfo
{
    IPvXAddress localAddr;
    int localPort = -1;
}

//
// Control info for connecting an UDP socket. To connect a socket,
// send a message to the ~UDP module with kind=UDP_C_CONNECT and an
// ~UDPConnectCommand attached. If the socket does not exist,
// it will be created.
//
// Both the address and the port must be filled in.
//
// @see ~UDPCommandCode
//
class UDPConnectCommand extends UDPControlInfo
{
    IPvXAddress remoteAddr;
    int remotePort = -1;
}


//
// Control info for sending data via UDP. To send a packet, send it to
// the ~UDP module with kind=UDP_C_SEND and an ~UDPSendCommand attached.
//
// UDP_C_SEND/~UDPSendCommand models both the Unix send() and sendto() calls.
// If the socket is connected, destAddr/destPort may be left blank (send()),
// otherwise it must contain the destination for the packet (sendto()).
//
// @see ~UDPCommandCode
//
class UDPSendCommand extends UDPControlInfo
{
    IPvXAddress destAddr; // destination IPv4 or IPv6 address
    int destPort = -1;     // destination port
    int interfaceId = -1; // interface on which the packet must be sent (see ~InterfaceTable)
}


//
// Control info for closing an UDP socket. To close a socket, send a message
// to the ~UDP module with kind=UDP_C_CLOSE and an ~UDPCloseCommand attached.
//
// @see ~UDPCommandCode
//
class UDPCloseCommand extends UDPControlInfo
{
}


//
// Control info that is attached to received data packets, sent up from the
// ~UDP module to the application with UDP_I_DATA as message kind.
//
// @see ~UDPCommandCode
//
class UDPDataIndication extends UDPControlInfo
{
    IPvXAddress srcAddr;   // source IPv4 or IPv6 address of the received packet
    IPvXAddress destAddr;  // destination IPv4 or IPv6 address of the received packet
    int srcPort;           // source port of the received packet
    int destPort;          // destination port of the received packet
    int ttl;               // TTL field of the received packet
    int interfaceId;       // interface on which the packet was received (see ~InterfaceTable)
    unsigned char typeOfService; // IPv4 Type of Service / IPv6 Traffic Class field of received packet
}

//
// Control info that is sent up from the ~UDP module to the application with
// UDP_I_ERROR as message kind when ~UDP receives an ICMP error for a packet
// previously sent from the socket.
//
// @see ~UDPCommandCode
//
class UDPErrorIndication extends UDPControlInfo
{
    IPvXAddress srcAddr;   // source IPv4 or IPv6 address of the undeliverable packet
    IPvXAddress destAddr;  // destination IPv4 or IPv6 address of the undeliverable packet
    int srcPort;           // source port of the undeliverable packet
    int destPort;          // destination port of the undeliverable packet
}

//
// Base class for UDP socket option control info classes.
//
class UDPSetOptionCommand extends UDPControlInfo
{
}

//
// Control info for setting the Time To Live (a.k.a. Hop Limit) option on an
// UDP socket. This option will affect both multicast and unicast packets.
// To set the option, send a message to the ~UDP module with kind=UDP_C_SETOPTION
// and an and instance of this control info class attached.
//
// @see ~UDPCommandCode
//
class UDPSetTimeToLiveCommand extends UDPSetOptionCommand
{
    int ttl;
}

//
// Control info for setting the Type of Service (IPv4) or Traffic Class (IPv6)
// header field on outgoing IPv4/IPv6 packets sent from an UDP socket.
// This is an 8-bit field, the lowest 6 bits are the DSCP (DiffServ Code Point)
// and the highest 2 bits are the ECN (Explicit Congestion Notification).
// To set the option, send a message to  the ~UDP module with kind=UDP_C_SETOPTION
// and an and instance of this control info class attached.
//
// @see ~UDPCommandCode
//
class UDPSetTypeOfServiceCommand extends UDPSetOptionCommand
{
    char tos;  // ToS byte for IPv4/IPv6
}

//
// Control info for setting the Broadcast option on an UDP socket.
// To set the option, send a message to the ~UDP module with kind=UDP_C_SETOPTION
// and an and instance of this control info class attached.
//
// @see ~UDPCommandCode
//
class UDPSetBroadcastCommand extends UDPSetOptionCommand
{
    bool broadcast;
}

//
// Control info for setting the multicast output interface for an UDP socket.
// To set the option, send a message to the ~UDP module with kind=UDP_C_SETOPTION
// and an and instance of this control info class attached.
//
// @see ~UDPCommandCode
//
class UDPSetMulticastInterfaceCommand extends UDPSetOptionCommand
{
    int interfaceId;
}

//
// Control info for setting the multicast loop option for an UDP socket.
// When the option is true, a copy of the outgoing multicast packet
// is delivered locally via the loopback interface.
// To set the option, send a message to the ~UDP module with kind=UDP_C_SETOPTION
// and an instance of this control info class attached.
//
// @see ~UDPCommandCode
//
class UDPSetMulticastLoopCommand extends UDPSetOptionCommand
{
    bool loop;
}

//
// Control info for letting an UDP socket join multicast groups.
// To set the option, send a message to the ~UDP module with kind=UDP_C_SETOPTION
// and an and instance of this control info class attached.
//
// The address-interfaceId pairs are passed as two separate arrays.
// When there are less interfaceIds than multicast addresses,
// then -1 is assumed (meaning join on all interfaces).
//
// @see ~UDPCommandCode
//
class UDPJoinMulticastGroupsCommand extends UDPSetOptionCommand
{
    IPvXAddress multicastAddr[];
    int interfaceId[];
}

//
//
// Control info for letting an UDP socket leave multicast groups.
// To set the option, send a message to the ~UDP module with kind=UDP_C_SETOPTION
// and an and instance of this control info class attached.
//
// @see ~UDPCommandCode
//
class UDPLeaveMulticastGroupsCommand extends UDPSetOptionCommand
{
    IPvXAddress multicastAddr[];
}

