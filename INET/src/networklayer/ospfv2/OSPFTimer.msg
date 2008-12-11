//
// Copyright (C) 2006 Andras Babos and Andras Varga
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
#include "INETDefs.h"
}}

enum OSPFTimerType
{

    InterfaceHelloTimer = 0;
    InterfaceWaitTimer = 1;
    InterfaceAcknowledgementTimer = 3;
    NeighborInactivityTimer = 4;
    NeighborPollTimer = 5;
    NeighborDDRetransmissionTimer = 6;
    NeighborUpdateRetransmissionTimer = 7;
    NeighborRequestRetransmissionTimer = 8;
    DatabaseAgeTimer = 9;
}

//
// Represents an OSPF Timer
//
packet OSPFTimer extends cMessage
{
    char timerKind enum(OSPFTimerType) = InterfaceHelloTimer;
}

