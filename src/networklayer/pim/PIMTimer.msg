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

class noncobject IPv4Address;

enum PIMTimerKind
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


// represents PIM Source Active Timer
message PIMkat
{
    IPv4Address 		source;          	// Source of multicast
    IPv4Address			group;				// Multicast group address
}

// represents PIM Register-stop timer
message PIMrst
{
    IPv4Address 		source;          	// Source of multicast
    IPv4Address			group;				// Multicast group address
}

// represents PIM Expiry timer
message PIMet
{
    IPv4Address			group;				// Multicast group address
    IPv4Address			source;
    int					StateType;
    int					intId;				// ID of interface
}

// represents PIM Join timer
message PIMjt
{
    IPv4Address			group;				// Multicast group address
	IPv4Address			JoinPruneAddr;		// Joined or Pruned IP Address
	IPv4Address			upstreamNbr;			// Address of upstream neigbor
	int					JoinType;				
}

message PIMppt
{
	IPv4Address			group;				// Multicast group address
	IPv4Address			JoinPruneAddr;		// Joined or Pruned IP Address
	IPv4Address			upstreamNbr;		// Address of upstream neigbor
	int					JoinPruneType;	
}