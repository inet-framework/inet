//
// Copyright (C) 2004 Andras Varga
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

//
// Andras Varga, 03/2004
//

cplusplus {{
#include "IPAddress.h"
}}

class noncobject IPAddress;

//
// NOT YET IN USE!
//
message TCPInterfacePacket extends cMessage
{
    fields:
        int tcpConnId;
        IPAddress srcAddr;
        IPAddress destAddr;
        int srcPort;
        int destPort;
        simtime_t timeout;
        int recPks;
        int numBitReq;
        int mss;


        //completePkt ??
        //label
        //origLength
//        originalLength
//        segLen
//        sequence
//        tcpFlagPsh
//        tcpFlagUrg
//        tcpheader
//        urgent

};
