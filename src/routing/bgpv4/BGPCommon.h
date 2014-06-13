//
// Copyright (C) 2010 Helene Lageber
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

#ifndef __INET_BGPCOMMON_H
#define __INET_BGPCOMMON_H

#include "IPv4Datagram.h"
//#include "TCPSocket.h"
#include "InterfaceEntry.h"

//Forward declarations:
class TCPSocket;

namespace BGP
{

const unsigned char TCP_PORT            = 179;

const unsigned char START_EVENT_KIND    = 81;
const unsigned char CONNECT_RETRY_KIND  = 82;
const unsigned char HOLD_TIME_KIND      = 83;
const unsigned char KEEP_ALIVE_KIND     = 89;
const unsigned char NB_TIMERS           = 4;
const unsigned char NB_STATS            = 6;
const unsigned char DEFAULT_COST        = 1;
const unsigned char NB_SESSION_MAX      = 255;

const unsigned char ROUTE_DESTINATION_CHANGED   = 90;
const unsigned char NEW_ROUTE_ADDED             = 91;
const unsigned char NEW_SESSION_ESTABLISHED     = 92;
const unsigned char ASLOOP_NO_DETECTED          = 93;
const unsigned char ASLOOP_DETECTED             = 94;

enum type {
        IGP         = 0,
        EGP         = 1,
        Incomplete  = 2
};

const unsigned char     AS_SET      = 1;
const unsigned char     AS_SEQUENCE = 2;

typedef IPv4Address       NextHop;
typedef unsigned short  ASID;
typedef unsigned long   SessionID;

struct SessionInfo{
    SessionID       sessionID;
    type            sessionType;
    ASID            ASValue;
    IPv4Address       routerID;
    IPv4Address       peerAddr;
    InterfaceEntry* linkIntf;
    TCPSocket*      socket;
    TCPSocket*      socketListen;
    bool            sessionEstablished;
};

} // namespace BGP

#endif

