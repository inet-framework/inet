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
// TCP commands, from application to TCP
//
enum TcpCommand
{
    TCP_C_OPEN_ACTIVE = 1;
    TCP_C_OPEN_PASSIVE = 2;
    TCP_C_SEND = 3;
    TCP_C_RECEIVE = 4;
    TCP_C_CLOSE = 5;
    TCP_C_ABORT = 6;
    TCP_C_STATUS = 7;
};

//
// TCP connection status, from TCP to application
//
enum TcpStatusInd
{
    TCP_I_NONE = 0;
    TCP_I_SEG_FWD = 1;
    TCP_I_RCVD_SYN = 2;
    TCP_I_ESTAB = 3;
    TCP_I_CLOSE_WAIT = 4;
    TCP_I_CLOSED = 5;
    TCP_I_ABORTED = 6;
};

//
// Control info for TCP connections. Travels between application and TCP.
//
message TCPInterfacePacket  // FIXME rename to TCPCommand and TCPIndication ?
{
    fields:
        int connId;
};

//
// Control info to be used for active or passive TCP open.
//
message TCPOpenCommand extends TCPInterfacePacket
{
    fields:
        IPAddress localAddr;
        IPAddress remoteAddr;
        int localPort;
        int remotePort;
        string sendQueueClass;
        string receiveQueueClass;
        string tcpAlgorithmClass;
};

