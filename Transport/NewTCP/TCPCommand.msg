//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


cplusplus {{
#include "IPAddress.h"
}}

class noncobject IPAddress;


//
// TCP command codes, from application to TCP
//
enum TcpCommandCode
{
    TCP_C_OPEN_ACTIVE = 1;
    TCP_C_OPEN_PASSIVE = 2;
    TCP_C_SEND = 3;
    TCP_C_CLOSE = 5;
    TCP_C_ABORT = 6;
    TCP_C_STATUS = 7;
};


//
// TCP connection status, from TCP to application
//
enum TcpStatusInd
{
    TCP_I_DATA = 1;             // data packet
    TCP_I_URGENT_DATA = 2;      // urgent data
    TCP_I_ESTABLISHED = 3;      // connection established
    TCP_I_CLOSING = 4;          // FIN received
    TCP_I_CLOSED = 5;           // connection in CLOSED
    TCP_I_CONNECTION_RESET = 6; // connection reset
};


enum TCPErrorCode
{
};


//
// Control info for TCP connections.
//
class TCPCommand
{
    properties:
        omitGetVerb = true;
    fields:
        int connId = -1;
};

//
// Control info for TCP connections.
//
class TCPErrorInfo extends TCPCommand
{
    properties:
        omitGetVerb = true;
    fields:
        int errorCode enum(TCPErrorCode);
        string message;
};

//
// Control info to be used for active or passive TCP open.
//
class TCPOpenCommand extends TCPCommand
{
    //properties:
    //    omitGetVerb = true;
    fields:
        IPAddress localAddr;
        IPAddress remoteAddr;
        int localPort = -1;
        int remotePort = -1;
        string sendQueueClass;
        string receiveQueueClass;
        string tcpAlgorithmClass;
};

//
// Control info to be used for TCP send.
//
message TCPSendCommand extends TCPCommand
{
    fields:
};


//
// Sent in response to TCP_C_STATUS. For explanation of variables,
// see RFC 793 or TCPStateVariables in TCPConnection.h.
//
class TCPStatusInfo extends TCPCommand
{
    properties:
        omitGetVerb = true;
    fields:
        int state;
        string stateName;

        IPAddress localAddr;
        IPAddress remoteAddr;
        int localPort;
        int remotePort;

        int snd_mss;

        unsigned int snd_una;
        unsigned int snd_nxt;
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
};

