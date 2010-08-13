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
    INTERFACE_HELLO_TIMER = 0;
    INTERFACE_WAIT_TIMER = 1;
    INTERFACE_ACKNOWLEDGEMENT_TIMER = 3;
    NEIGHBOR_INACTIVITY_TIMER = 4;
    NEIGHBOR_POLL_TIMER = 5;
    NEIGHBOR_DD_RETRANSMISSION_TIMER = 6;
    NEIGHBOR_UPDATE_RETRANSMISSION_TIMER = 7;
    NEIGHBOR_REQUEST_RETRANSMISSION_TIMER = 8;
    DATABASE_AGE_TIMER = 9;
}

//
// Represents an OSPF Timer
//
packet OSPFTimer extends cMessage
{
    char timerKind enum(OSPFTimerType) = INTERFACE_HELLO_TIMER;
}

