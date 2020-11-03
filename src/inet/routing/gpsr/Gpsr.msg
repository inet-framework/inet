//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

import inet.common.INETDefs;
import inet.common.TlvOptions;
import inet.common.geometry.Geometry;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.L3Address;

cplusplus {{
#include "inet/routing/gpsr/GpsrDefs.h"
}}

namespace inet;

enum GpsrForwardingMode {
    GPSR_GREEDY_ROUTING = 1;
    GPSR_PERIMETER_ROUTING = 2;
};

enum GpsrPlanarizationMode {
    GPSR_NO_PLANARIZATION = 0;
    GPSR_GG_PLANARIZATION = 1;
    GPSR_RNG_PLANARIZATION = 2;
};

//
// The GPSR beacon packet is sent periodically by all GPSR routers to notify
// the neighbors about the router's address and position.
//
class GpsrBeacon extends FieldsChunk
{
    L3Address address;
    Coord position;
}

//
// The GPSROption is used to add extra routing information for network datagrams.
//
// For more information, see the GPSR paper
// http://www.eecs.harvard.edu/~htk/publication/2000-mobi-karp-kung.pdf
//
class GpsrOption extends TlvOptionBase {
    GpsrForwardingMode routingMode;
    Coord destinationPosition;               // D
    Coord perimeterRoutingStartPosition;     // Lp
    Coord perimeterRoutingForwardPosition;   // Lf
    L3Address currentFaceFirstSenderAddress;   // e0
    L3Address currentFaceFirstReceiverAddress; // e0
    L3Address senderAddress; // TODO: this field is not strictly needed by GPSR (should be eliminated)
}

