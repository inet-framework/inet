//
// Copyright (C) 2013 Opensim Ltd.
// Author: Levente Meszaros
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

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.L3Address;

cplusplus {{
#include "inet/routing/dymo/DymoDefs.h"
}}

namespace inet::dymo;

// TODO: metric types are defined in a separate [RFC 6551]
enum DymoMetricType {
    HOP_COUNT = 3;    // Hop Count has Metric Type assignment 3
};

class DymoSequenceNumber
{
    @existingClass;
    @opaque;
    @byValue;
    @fromString(string2ulong($));
    @toString(ulong2string($));
    @defaultValue(0);
    @toValue((int)$);
    @fromValue($.intValue());
}

//
// classes
//

class AddressBlock extends cObject
{
    L3Address address;
    int prefixLength;
    bool hasValidityTime;
    simtime_t validityTime;
    bool hasMetric;
    double metric;
    bool hasMetricType;
    DymoMetricType metricType;
    bool hasSequenceNumber;
    DymoSequenceNumber sequenceNumber = 0;
}

//
// Dymo timer messages
//

message RreqTimer {
    L3Address target;
}

message RreqWaitRrepTimer extends RreqTimer {
    int retryCount;
}

message RreqBackoffTimer extends RreqTimer {
    int retryCount;
}

message RreqHolddownTimer extends RreqTimer {
}

//
// Dymo packets
//

class DymoPacket extends FieldsChunk {
    int hopLimit;
}

class RteMsg extends DymoPacket {
    int hopCount;
    AddressBlock originatorNode;
    AddressBlock targetNode;
    AddressBlock addedNode[];
}

//
// Dymo RREQ packet
//
class Rreq extends RteMsg {
}

//
// Dymo RREP packet
//
class Rrep extends RteMsg {
}

//
// Dymo RREP ACK packet
//
class RrepAck extends DymoPacket {
}

//
// Dymo RERR packet
//
class Rerr extends DymoPacket {
    AddressBlock unreachableNode[];
}
