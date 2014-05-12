//
// Copyright (C) 2014 OpenSim Ltd.
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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

cplusplus {{
#include "IPv4Address.h"
#include "simtime_t.h"
}}

class noncobject IPv4Address;

//
// Helper struct to represent (IPv4Address, sequence number) pairs in a RERR message
//
struct UnreachableNode
{
    IPv4Address addr;
    unsigned int seqNum;
};

enum AODVControlPacketType
{
    RREQ = 1;
    RREP = 2;
    RERR = 3;
    RREPACK = 4;
};

//
// Base packet for AODV Control Packets
//
packet AODVControlPacket
{
    unsigned int packetType;
}

//
// Represents an AODV Route Request
//
class AODVRREQ extends AODVControlPacket
{
    unsigned int packetType = RREQ;
    bool joinFlag;
    bool repairFlag;
    bool gratuitousRREPFlag;
    bool destOnlyFlag;
    bool unknownSeqNumFlag;
    unsigned int hopCount;
    unsigned int rreqId;
    IPv4Address destAddr;
    unsigned int destSeqNum;
    IPv4Address originatorAddr;
    unsigned int originatorSeqNum;
}

//
// Represents an AODV Route Reply
//
class AODVRREP extends AODVControlPacket
{
    unsigned int packetType = RREP;
    bool repairFlag;
    bool ackRequiredFlag;
    unsigned int prefixSize;
    unsigned int hopCount;
    IPv4Address destAddr;
    unsigned int destSeqNum;
    IPv4Address originatorAddr;
    unsigned int originatorSeqNum;
    simtime_t lifeTime;
}

//
// Represents an AODV Route Error
//
class AODVRERR extends AODVControlPacket
{
    unsigned int packetType = RERR;
    UnreachableNode unreachableNodes[];
    bool noDeleteFlag;
    unsigned int destCount;
}

//
// Represents an AODV Route Reply ACK
//
class AODVRREPACK extends AODVControlPacket
{
    unsigned int packetType = RREPACK;
}

//
// Represents a timer for a Route Reply packet
//
message WaitForRREP
{
    IPv4Address destAddr;
    unsigned int lastTTL;
    bool fromInvalidEntry;
}
