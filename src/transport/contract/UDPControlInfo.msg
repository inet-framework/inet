//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
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
// \UDP command codes, sent by the application to UDP. These constants
// should be set as message kind on a message sent to the UDP entity.
//
// @see UDPControlInfo, UDPStatusInd, UDP
//
enum UDPCommandCode
{

    UDP_C_DATA = 0;    // datagram to send (must carry UDPControlInfo)
    UDP_C_BIND = 1;    // fill srcPort and srcAddr in UDPControlInfo
    UDP_C_CONNECT = 2; // fill destAddr and destPort in UDPControlInfo
    UDP_C_UNBIND = 3;  // fill sockId in UDPControlInfo
}

//
// \UDP indications, sent by UDP to the application. UDP will set these
// constants as message kind on messages it sends to the application.
//
// @see UDPControlInfo, UDPCommandCode, UDP
//
enum UDPStatusInd
{

    UDP_I_DATA = 0;  // data packet (set on data packet)
    UDP_I_ERROR = 1; // ICMP error received on a sent datagram
}


//
// Control info for sending/receiving data via \UDP. Travels between
// application and the UDP module.
//
// The most convenient way to handle UDP is the UDPSocket class, which hides
// UDPControlInfo completely from you. But even if you manage talking
// to UDP yourself without help from UDPSocket, sockId must be obtained
// from UDPSocket::generateSocketId().
//
//# TODO explain userId, sockId, etc.
//
// @see UDPCommandCode
//
class UDPControlInfo
{
    int sockId = -1;   // uniquely identifies the \UDP socket
    int userId = -1;   // id than can be freely used by the app
    IPvXAddress srcAddr;  // source \IP or \IPv6 address, or local address with BIND
    IPvXAddress destAddr; // destination \IP or \IPv6 address
    int srcPort;   // \UDP source port in packet, or local port with BIND
    int destPort;  // \UDP destination port in packet
    int interfaceId = -1; // interface on which pk was received/should be sent (see InterfaceTable)
}

