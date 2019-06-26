//
// Copyright (C) 2019 Universidad de Malaga
// Author: Alfonso Ariza
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
import inet.common.TlvOptions;

namespace inet::inetmanet;

//
// Helper struct to represent (L3Address, sequence number) pairs in a RERR message
//
struct UnreachableNode
{
    L3Address addr;
    unsigned int seqNum;
};

class NeigborData {
    L3Address addr;
    bool pendingConfirmation; 
    bool isBidir;
    unsigned int seqNum;
    uint8_t metric;
    uint8_t numHelloRec;
}

enum LoadNgControlPacketType
{    
    RREQ = 1;
    RREP = 2;
    RERR = 3;
    RREPACK = 4;
    HELLO = 5;
    HELLOROOT = 6;
};

enum LoadNgControlMetrics
{
    Metric1 = 1;    
};

enum LoadNgControl
{
    REQACK = 125;
    METRIC = 124;
    ADDRTYPE = 128;
    FLAGS = 129;
    SINK = 130;
};

enum AddrTypeCodes
{
    DESTINATION = 0;
    ERRORCODE = 1;
};

enum MetricExntesion
{
    HOPCOUNT = 0;
    DIMENSIONLESS = 1;
};


class LoadNgOption extends TlvOptionBase {
    bool extensionFlag = false;
    bool indexStartFlag = false;
    bool indexStopFlag = false;
    bool valueFlag = false;
}


class LoadNgMetricOption extends LoadNgOption { // size 6 B
    type = METRIC;
    length = 2;   // total length of option
    valueFlag = true;
    extensionFlag = true;
    unsigned char extension = HOPCOUNT;
    unsigned int value; 
}


class LoadNgAckRrepReq extends LoadNgOption { // size  4
    type = FLAGS;
    valueFlag = true;    
    length = 1;   // total length of option
    short value = REQACK;  
}

class AddressTlv extends TlvOptionBase { // size 2 
    type = ADDRTYPE;
    bool indexStartFlag = false;
    bool indexStopFlag = false;
    bool valueFlag = false;
    bool typeExtFlag = false;
}

class AddressTlvError extends AddressTlv {   // size 6 B
    length = 2;   // total length of option
    typeExtFlag = true;
    unsigned char typeExtension = ERRORCODE;
    short value = 0;
}


class AddressBlock extends TlvOptionBase {   //2 + (num address * size address)
    bool indexStartFlag = false;
    bool indexStopFlag = false;
    bool valueFlag = false;
    L3Address address[];
}


class LoadNgSink extends LoadNgOption { // size 6 B
    type = SINK;
    length = 8;   // total length of option
    uint8_t hops;
    uint8_t metric; 
    L3Address addressSink; 
}

//
// Base packet for AODV Control Packets
//
class LoadNgControlPacket extends FieldsChunk
{
    unsigned int packetType;
    // flag field
    bool origAddr = false;
    bool hopLimits = false;
    bool hopCounts = false;
    bool seqField = false;
    unsigned int addrLen;
    B messageLength;
}

//
// Represents an AODV Route Request
//
class Rreq extends LoadNgControlPacket
{
    packetType = RREQ;
    origAddr = true;
    hopLimits = true;
    hopCounts = true;
    seqField = true;
    addrLen = 3;
    L3Address originatorAddr;
    unsigned int hopLimit;
    unsigned int hopCount;
    unsigned int seqNum;
    TlvOptions tlvOptions;
    L3Address destAddr; // it should be a block of type TLV + 2 B
    TlvOptions addressTlvOptions;  
}

//
// Represents an AODV Route Reply
//
class Rrep extends LoadNgControlPacket
{
    packetType = RREP;
    origAddr = true;
    hopLimits = true;
    hopCounts = true;
    seqField = true;
    addrLen = 3;
    L3Address originatorAddr;
    unsigned int hopLimit;
    unsigned int hopCount;
    unsigned int seqNum;
    TlvOptions tlvOptions;
    L3Address destAddr; // it should be a block of type TLV + 2 B
    TlvOptions addressTlvOptions;
}

//
// Represents an AODV Route Error
//
class Rerr extends LoadNgControlPacket
{
    packetType = RERR;
    origAddr = true;
    hopLimits = true;
    hopCounts = false;
    seqField = false;
    addrLen = 3;
    unsigned int hopLimit;
    TlvOptions tlvOptions;
    UnreachableNode unreachableNodes[]; // it should be a block of type TLV + 2 B    
    TlvOptions addressTlvOptions;
}

//
// Represents an AODV Route Reply ACK
//
class RrepAck extends LoadNgControlPacket
{
    packetType = RREPACK;
    origAddr = true;
    hopLimits = false;
    hopCounts = false;
    seqField = false;
    addrLen = 3;
    L3Address originatorAddr;
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

class Hello extends LoadNgControlPacket
{
    packetType = HELLO;
    origAddr = true;
    hopLimits = true;
    hopCounts = true;
    seqField = true;
    addrLen = 3;
    uint64_t helloIdentifier;
    L3Address originatorAddr;
    unsigned int hopLimit;
    unsigned int hopCount;
    unsigned int seqNum;
    simtime_t lifetime;
    TlvOptions tlvOptions;
    NeigborData neighAddrs[]; // it should be a block of type TLV + 2 B
    TlvOptions addressTlvOptions;  
}

