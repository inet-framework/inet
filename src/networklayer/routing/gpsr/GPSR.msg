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

cplusplus {{
#include "Address.h"
#include "Coord.h"
#include "GPSRDefs.h"
}}

class noncobject Address;

class noncobject Coord;

class noncobject GPSRForwardingMode;

//
// GPSR beacon packet is sent periodically by all GPSR routers to notify
// the neighbors about the router's address and position. 
//
packet GPSRBeacon {
    Address address;
    Coord position;
}

//
// GPSR packet is used to add extra routing information for routed packets.
// KLUDGE: this packet is now used to wrap the content of the IP datagram
// KLUDGE: we should rather add these fields as IP header extension
//
// For more information, see the GPSR paper at
// http://www.eecs.harvard.edu/~htk/publication/2000-mobi-karp-kung.pdf
//
packet GPSRPacket {
    GPSRForwardingMode routingMode;
    Coord destinationPosition;               // D
    Coord perimeterRoutingStartPosition;     // Lp
    Coord perimeterRoutingForwardPosition;   // Lf
    Address currentFaceFirstSenderAddress;   // e0
    Address currentFaceFirstReceiverAddress; // e0
    Address senderAddress; // TODO: this field is not strictly needed by GPSR (eliminate)
}
