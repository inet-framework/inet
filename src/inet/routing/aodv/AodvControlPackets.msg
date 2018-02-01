//
// Copyright (C) 2014 OpenSim Ltd.
// Author: Benjamin Seregi
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

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.L3Address;

namespace inet::aodv;

//
// Helper struct to represent (L3Address, sequence number) pairs in a RERR message
//
struct UnreachableNode
{
    L3Address addr;
    unsigned int seqNum;
};

enum AodvControlPacketType
{
    RREQ = 1;
    RREP = 2;
    RERR = 3;
    RREPACK = 4;
};

//
// Base packet for AODV Control Packets
//
class AodvControlPacket extends FieldsChunk
{
    unsigned int packetType;
}

//
// Represents an AODV Route Request
//
class Rreq extends AodvControlPacket
{
    packetType = RREQ;
    bool joinFlag;
    bool repairFlag;
    bool gratuitousRREPFlag;
    bool destOnlyFlag;
    bool unknownSeqNumFlag;
    unsigned int hopCount;
    unsigned int rreqId;
    L3Address destAddr;
    unsigned int destSeqNum;
    L3Address originatorAddr;
    unsigned int originatorSeqNum;
}

//
// Represents an AODV Route Reply
//
class Rrep extends AodvControlPacket
{
    packetType = RREP;
    bool repairFlag;
    bool ackRequiredFlag;
    unsigned int prefixSize;
    unsigned int hopCount;
    L3Address destAddr;
    unsigned int destSeqNum;
    L3Address originatorAddr;
    unsigned int originatorSeqNum;
    simtime_t lifeTime;
}

//
// Represents an AODV Route Error
//
class Rerr extends AodvControlPacket
{
    packetType = RERR;
    UnreachableNode unreachableNodes[];
    bool noDeleteFlag;
    unsigned int destCount;
}

//
// Represents an AODV Route Reply ACK
//
class RrepAck extends AodvControlPacket
{
    packetType = RREPACK;
}

//
// Represents a timer for a Route Reply packet
//
message WaitForRrep
{
    L3Address destAddr;
    unsigned int lastTTL;
    bool fromInvalidEntry;
}
