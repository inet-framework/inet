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

cplusplus {{
#include "inet/networklayer/common/L3Address.h"
#include "inet/routing/dymo/DYMOdefs.h"
}}

namespace inet::dymo;

class noncobject L3Address;

class noncobject DYMOSequenceNumber;

class noncobject DYMOMetricType;

//
// classes
//

class AddressBlock {
    L3Address address;
    int prefixLength;
    bool hasValidityTime;
    simtime_t validityTime;
    bool hasMetric;
    double metric;
    bool hasMetricType;
    DYMOMetricType metricType;
    bool hasSequenceNumber;
    DYMOSequenceNumber sequenceNumber;
}

//
// DYMO timer messages
//

message RREQTimer {
    L3Address target;
}

message RREQWaitRREPTimer extends RREQTimer {
    int retryCount;
}

message RREQBackoffTimer extends RREQTimer {
    int retryCount;
}

message RREQHolddownTimer extends RREQTimer {
}

//
// DYMO packets
//

packet DYMOPacket {
    int hopLimit;
}

packet RteMsg extends DYMOPacket {
    int hopCount;
    AddressBlock originatorNode;
    AddressBlock targetNode;
    AddressBlock addedNode[];
}

//
// DYMO RREQ packet
//
packet RREQ extends RteMsg {
}

//
// DYMO RREP packet
//
packet RREP extends RteMsg {
}

//
// DYMO RREP ACK packet
//
packet RREP_ACK extends DYMOPacket {
}

//
// DYMO RERR packet
//
packet RERR extends DYMOPacket {
    AddressBlock unreachableNode[];
}
