//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
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
// Authors: Veronika Rybova, Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)

cplusplus {{
#include "IPv4Address.h"
}};

//struct IPAddress;
class noncobject IPv4Address;
//class pimSplitter extends void;

enum PIMTimerType
{
    // global timers
   HelloTimer = 1;
   TriggeredHelloDelay = 2;
   
   // timers for each interface and each neighbor (N,I)
   NeighborLivenessTimer = 3;
   
   // timers for each interface and each source-group pair (S,G,I)
   AssertTimer = 4;
   PruneTimer = 5;
   PrunePendingTimer = 6;
   
   // timers for each source-group pair (S,G)
   GraftRetryTimer = 7;
   UpstreamOverrideTimer = 8;
   PruneLimitTimer = 9;
   SourceActiveTimer = 10;
   StateRefreshTimer = 11;
   
   //PIM-SM specific timers
   KeepAliveTimer = 12;
   RegisterStopTimer = 13;
   ExpiryTimer = 14;
   JoinTimer = 15;
}


// Represents an PIM Timer
packet PIMTimer extends cMessage
{
      char 				timerKind enum(PIMTimerType);
}

// Represents an PIM Neighbor Livness Timer
packet PIMnlt extends PIMTimer
{
    timerKind = NeighborLivenessTimer;
    int 				ntId;          	// Identification of entry in Neigbor table
}

// represents PIM Prune Timer
packet PIMpt extends PIMTimer
{
    timerKind = PruneTimer;
    IPv4Address 		source;          	// Source of multicast
    IPv4Address			group;				// Multicast group address
    int					intId;				// ID of interface
}

// represents PIM Graft Retry Timer
packet PIMgrt extends PIMTimer
{
    timerKind = GraftRetryTimer;
    IPv4Address 		source;          	// Source of multicast
    IPv4Address			group;				// Multicast group address
}

// represents PIM Source Active Timer
packet PIMsat extends PIMTimer
{
    timerKind = SourceActiveTimer;
    IPv4Address 		source;          	// Source of multicast
    IPv4Address			group;				// Multicast group address
}

// represents PIM State Refresh Timer
packet PIMsrt extends PIMTimer
{
    timerKind = StateRefreshTimer;
    IPv4Address 		source;          	// Source of multicast
    IPv4Address			group;				// Multicast group address
}

// represents PIM Source Active Timer
packet PIMkat extends PIMTimer
{
    timerKind = KeepAliveTimer;
    IPv4Address 		source;          	// Source of multicast
    IPv4Address			group;				// Multicast group address
}

// represents PIM Register-stop timer
packet PIMrst extends PIMTimer
{
    timerKind = RegisterStopTimer;
    IPv4Address 		source;          	// Source of multicast
    IPv4Address			group;				// Multicast group address
}

// represents PIM Expiry timer
packet PIMet extends PIMTimer
{
    timerKind = ExpiryTimer;
    IPv4Address			group;				// Multicast group address
    IPv4Address			source;
    int					StateType;
    int					intId;				// ID of interface
}

// represents PIM Join timer
packet PIMjt extends PIMTimer
{
    timerKind = JoinTimer;
    IPv4Address			group;				// Multicast group address
	IPv4Address			JoinPruneAddr;		// Joined or Pruned IP Address
	IPv4Address			upstreamNbr;			// Address of upstream neigbor
	int					JoinType;				
}

packet PIMppt extends PIMTimer
{
	timerKind = PrunePendingTimer;
	IPv4Address			group;				// Multicast group address
	IPv4Address			JoinPruneAddr;		// Joined or Pruned IP Address
	IPv4Address			upstreamNbr;		// Address of upstream neigbor
	int					JoinPruneType;	
}