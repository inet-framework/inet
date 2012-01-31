//
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
// TCP command codes, sent by the application to TCP. These constants
// should be set as message kind on a message sent to the TCP entity.
//
// @see ~TCPCommand, ~TCPOpenCommand, ~ITCP
//
enum TcpCommandCode
{
    TCP_C_OPEN_ACTIVE = 1;   // active open (must carry ~TCPOpenCommand)
    TCP_C_OPEN_PASSIVE = 2;  // passive open (must carry ~TCPOpenCommand)
    TCP_C_SEND = 3;          // send data (set on data packet)
    TCP_C_CLOSE = 5;         // "I have no more data to send"
    TCP_C_ABORT = 6;         // abort connection
    TCP_C_STATUS = 7;        // request status info (TCP_I_STATUS) from TCP
}


//
// TCP indications, sent by TCP to the application. TCP will set these
// constants as message kind on messages it sends to the application.
//
// @see ~TCPCommand, ~TCPStatusInfo, ~ITCP
//
enum TcpStatusInd
{
    TCP_I_DATA = 1;              // data packet (set on data packet)
    TCP_I_URGENT_DATA = 2;       // urgent data (set on data packet)
    TCP_I_ESTABLISHED = 3;       // connection established
    TCP_I_PEER_CLOSED = 4;       // FIN received from remote TCP
    TCP_I_CLOSED = 5;            // connection closed normally (via FIN exchange)
    TCP_I_CONNECTION_REFUSED = 6; // connection refused
    TCP_I_CONNECTION_RESET = 7;  // connection reset
    TCP_I_TIMED_OUT = 8;         // conn-estab timer went off, or max retransm. count reached
    TCP_I_STATUS = 9;            // status info (will carry ~TCPStatusInfo)
}


//
// Currently not in use.
//
enum TCPErrorCode
{
}


//
// Control info for TCP connections. This class is to be set as control info
// (see cMessage::setControlInfo()) on all messages exchanged between TCP and
// application, in both directions. Some commands and indications
// (TCP_C_OPEN_xxx, TCP_I_STATUS) use subclasses.
//
// connId identifies the connection locally within the application (internally,
// TCP uses the (app gate index, connId) pair to identify the socket).
// connId is to be chosen by the application in the open command.
//
//# TODO explain userId
//
// @see ~TcpCommandCode, ~TcpStatusInd, ~TCPOpenCommand, ~TCPStatusInfo, ~ITCP
//
class TCPCommand
{
    int connId = -1;   // identifies the socket within the application
    int userId = -1;   // id than can be freely used by the app
}


//
// Currently not in use.
//
class TCPErrorInfo extends TCPCommand
{
    int errorCode enum(TCPErrorCode);
    string messageText;
}

//
// Defines what to transmit as payload in TCP segments
//
// Currently you have the following choices:
//   - TCP_TRANSFER_BYTECOUNT:
//      TCP layer will transmit byte counts only.
//   - TCP_TRANSFER_OBJECT:
//      TCP layer will transmit the copy of application packet C++ objects
//   - TCP_TRANSFER_BYTESTREAM:
//      TCP layer will transmit bytes of the application packet.
//
// See ~ITCP (the TCP layer interface) for details.
//
enum TCPDataTransferMode
{
    TCP_TRANSFER_UNDEFINED = 0;         // Invalid value
    TCP_TRANSFER_BYTECOUNT = 1;         // Transmit byte counts only
    TCP_TRANSFER_OBJECT = 2;            // Transmit the application packet C++ objects
    TCP_TRANSFER_BYTESTREAM = 3;        // Transmit raw bytes
}

//
// Control info to be used for active or passive TCP open.
//
// localAddr, remoteAddr, localPort, remotePort should be self-explanatory.
// localAddr is optional because TCP can learn it from IP when a packet
// is received from the peer; localPort is optional because TCP supports
// ephemeral ports.
//
// The fork parameter is used with passive open, and controls what happens
// when an incoming connection is received. With fork=true, it emulates
// the Unix accept(2) syscall semantics: a new connection structure
// is created for the connection (with a new connId, see in ~TCPCommand),
// and the connection structure with the old connId remains listening.
// With fork=false, all the above does not happen: the first connection
// is accepted (with the original connId), and further incoming connections
// will be refused by TCP by sending an RST segment.
//
// The dataTransferMode and tcpAlgorithmClass fields
// allow per-connection TCP configuration.
// The dataTransferMode field set the 
// The tcpAlgorithmClass field may contain name of class subclassed from
// TCPAlgorithm, respectively.
// If not set, module parameters with similar names are used.
//
// @see ~TcpCommandCode, ~ITCP
//
class TCPOpenCommand extends TCPCommand
{
    IPvXAddress localAddr; // may be left empty
    IPvXAddress remoteAddr;// required for active open
    int localPort = -1;       // required for passive open
    int remotePort = -1;      // required for active open
    bool fork = false;        // used only for passive open
    int dataTransferMode enum(TCPDataTransferMode); // whether to transmit C++ objects, real bytes or just byte counts. See ~TCPDataTransferMode.
    string tcpAlgorithmClass; // TCP congestion control algorithm; leave empty for default
}


//
// Control info to be used with the SEND command.
//
// @see ~TcpCommandCode, ~ITCP
//
class TCPSendCommand extends TCPCommand
{
}


//
// Sent with message kind TCP_I_ESTABLISHED, to let the app know
// about the local and remote IP address and port.
//
// @see ~TcpCommandCode, ~ITCP
//
class TCPConnectInfo extends TCPCommand
{
    IPvXAddress localAddr;
    IPvXAddress remoteAddr;
    int localPort;
    int remotePort;
}


//
// Sent with message kind TCP_I_STATUS, in response to command TCP_C_STATUS.
// For explanation of variables, see RFC 793 or TCPStateVariables in
// TCPConnection.h.
//
// @see ~TcpStatusInd, ~TcpCommandCode, ~ITCP
//
class TCPStatusInfo extends TCPCommand
{
    int state;
    string stateName;

    IPvXAddress localAddr;
    IPvXAddress remoteAddr;
    int localPort;
    int remotePort;

    unsigned int snd_mss;

    unsigned int snd_una;
    unsigned int snd_nxt;
    unsigned int snd_max;
    unsigned int snd_wnd;
    unsigned int snd_up;
    unsigned int snd_wl1;
    unsigned int snd_wl2;
    unsigned int iss;

    unsigned int rcv_nxt;
    unsigned int rcv_wnd;
    unsigned int rcv_up;
    unsigned int irs;

    bool fin_ack_rcvd;
}

