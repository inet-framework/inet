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


import inet.common.INETDefs;
import inet.networklayer.common.IpProtocolId;
import inet.networklayer.common.L3Address;

namespace inet;

//
// TCP command codes, sent by the application to TCP. These constants
// should be set as message kind on a message sent to the TCP entity.
//
// @see ~TcpCommand, ~TcpOpenCommand, ~ITcp
//
enum TcpCommandCode
{
    TCP_C_OPEN_ACTIVE = 1;   // active open (must carry ~TcpOpenCommand)
    TCP_C_OPEN_PASSIVE = 2;  // passive open (must carry ~TcpOpenCommand)
    TCP_C_ACCEPT = 3;        // accept connection
    TCP_C_SEND = 4;          // send data (set on data packet)
    TCP_C_CLOSE = 5;         // "I have no more data to send"
    TCP_C_ABORT = 6;         // abort connection
    TCP_C_STATUS = 7;        // request status info (TCP_I_STATUS) from TCP
    TCP_C_QUEUE_BYTES_LIMIT = 8; // set send queue limit (in bytes)
    TCP_C_READ = 9;          // send request to TCP to deliver data
    TCP_C_DESTROY = 10;      // send request to TCP to destroy the socket descriptor
    TCP_C_SETOPTION = 11;    // setting TCP socket options
}

//
// TCP indications, sent by TCP to the application. TCP will set these
// constants as message kind on messages it sends to the application.
//
// @see ~TcpCommand, ~TcpStatusInfo, ~ITcp
//
enum TcpStatusInd
{
    TCP_I_DATA = 1;              // data packet (set on data packet)
    TCP_I_URGENT_DATA = 2;       // urgent data (set on data packet)
    TCP_I_AVAILABLE = 3;         // conncetion available
    TCP_I_ESTABLISHED = 4;       // connection established
    TCP_I_PEER_CLOSED = 5;       // FIN received from remote TCP
    TCP_I_CLOSED = 6;            // connection closed normally (via FIN exchange)
    TCP_I_CONNECTION_REFUSED = 7; // connection refused
    TCP_I_CONNECTION_RESET = 8;  // connection reset
    TCP_I_TIMED_OUT = 9;         // conn-estab timer went off, or max retransm. count reached
    TCP_I_STATUS = 10;           // status info (will carry ~TcpStatusInfo)
    TCP_I_SEND_MSG = 11;         // send queue abated, send more messages
    TCP_I_DATA_NOTIFICATION = 12; // notify the upper layer that data has arrived
}

//
// Currently not in use.
//
enum TcpErrorCode
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
// @see ~TcpCommandCode, ~TcpStatusInd, ~TcpOpenCommand, ~TcpStatusInfo, ~ITcp
//
class TcpCommand extends cObject
{
    int userId = -1;   // id than can be freely used by the app
}

//
// Currently not in use.
//
class TcpErrorInfo extends TcpCommand
{
    TcpErrorCode errorCode;
    string messageText;
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
// is created for the connection (with a new connId, see in ~TcpCommand),
// and the connection structure with the old connId remains listening.
// With fork=false, all the above does not happen: the first connection
// is accepted (with the original connId), and further incoming connections
// will be refused by TCP by sending an RST segment.
//
// The tcpAlgorithmClass field allows per-connection TCP configuration.
// The tcpAlgorithmClass field may contain name of class subclassed from
// TcpAlgorithm, respectively.
// If not set, module parameters with similar names are used.
//
// @see ~TcpCommandCode, ~ITcp
//
class TcpOpenCommand extends TcpCommand
{
    L3Address localAddr; // may be left empty
    L3Address remoteAddr;// required for active open
    int localPort = -1;       // required for passive open
    int remotePort = -1;      // required for active open
    bool fork = false;        // used only for passive open
    string tcpAlgorithmClass; // TCP congestion control algorithm; leave empty for default
}

//
// Control info to be used to accept an available connection.
//
class TcpAcceptCommand extends TcpCommand
{
}

//
// Base class for TCP socket option control info classes.
//
class TcpSetOptionCommand extends TcpCommand
{
}

class TcpSetTimeToLiveCommand extends TcpSetOptionCommand
{
    int ttl;
}

class TcpSetDscpCommand extends TcpSetOptionCommand
{
    short dscp;   // dscp for Ipv4/Ipv6
}

class TcpSetTosCommand extends TcpSetOptionCommand
{
    short tos;   // type of service for Ipv4 / traffic class for Ipv6
}

//
// Sent with message kind TCP_I_AVAILABLE, to let the app know
// about the local and remote IP address and port.
//
// @see ~TcpCommandCode, ~ITcp
//
class TcpAvailableInfo extends TcpCommand
{
    L3Address localAddr;
    L3Address remoteAddr;
    int localPort;
    int remotePort;
    int newSocketId;
}

//
// Sent with message kind TCP_I_ESTABLISHED, to let the app know
// about the local and remote IP address and port.
//
// @see ~TcpCommandCode, ~ITcp
//
class TcpConnectInfo extends TcpCommand
{
    L3Address localAddr;
    L3Address remoteAddr;
    int localPort;
    int remotePort;
}

//
// Sent with message kind TCP_I_STATUS, in response to command TCP_C_STATUS.
// For explanation of variables, see RFC 793 or TcpStateVariables in
// TcpConnection.h.
//
// @see ~TcpStatusInd, ~TcpCommandCode, ~ITcp
//
class TcpStatusInfo extends TcpCommand
{
    int state;
    string stateName;

    L3Address localAddr;
    L3Address remoteAddr;
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

