//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.L3Address;

cplusplus {{
#include "inet/routing/dymo/DymoDefs.h"
}}

namespace inet::dymo;

// TODO metric types are defined in a separate [RFC 6551]
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
    @packetData;
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

message PacketJitterTimer{
    cPacket *jitteredPacket @owned;
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
