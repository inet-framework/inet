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
// TCP connection status information (TCP -> Application)
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
message TCPInterfacePacket
{
    fields:
        int connId;
        IPAddress srcAddr;
        IPAddress destAddr;
        int srcPort;
        int destPort;
        //...
};

