//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
// Copyright (C) 2019 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

import inet.common.INETDefs;
import inet.common.Units;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.L3Address;
import inet.transportlayer.common.CrcMode;

namespace inet;

cplusplus {{
const B PIM_HEADER_LENGTH = B(4);
const B ENCODED_UNICODE_ADDRESS_LENGTH = B(6); // Ipv4 only
const B ENCODED_GROUP_ADDRESS_LENGTH = B(8); // Ipv4 only
const B ENCODED_SOURCE_ADDRESS_LENGTH = B(8); // Ipv4 only
}}

enum PimPacketType
{
   Hello = 0;
   Register = 1;
   RegisterStop = 2;
   JoinPrune = 3;
   Bootstrap = 4;
   Assert = 5;
   Graft = 6;
   GraftAck = 7;
   CandidateRPAdvertisement = 8;
   StateRefresh = 9;    // in RFC 3973
}

enum PimHelloOptionType
{
   Holdtime = 1;
   LANPruneDelay = 2;
   DRPriority = 19;
   GenerationID = 20;
   StateRefreshCapable = 21;
   AddressList = 24;
}

struct EncodedUnicastAddress
{
    @packetData;
    L3Address unicastAddress;
}

struct EncodedGroupAddress
{
    @packetData;
    bool        B;
    short       reserved = 0;
    bool        Z;
    short       maskLength = 32;    //TODO Remove initializer, 32 for IPv4 only!!!
    L3Address   groupAddress;
}

struct EncodedSourceAddress
{
    @packetData;
    short       reserved = 0;
    bool        S;
    bool        W;                //WC bit
    bool        R;                //RPT bit
    short       maskLength = 32;    //TODO Remove initializer, 32 for IPv4 only!!!
    L3Address   sourceAddress;
}

class HelloOption extends cObject
{
    @packetData;
    PimHelloOptionType type;
}

class HoldtimeOption extends HelloOption
{
    type = Holdtime;
    uint16_t holdTime; // (in seconds)
}

class LanPruneDelayOption extends HelloOption
{
    type = LANPruneDelay;
    bool     T = false;
    uint16_t propagationDelay;
    uint16_t overrideInterval;
}

class DrPriorityOption extends HelloOption
{
    type = DRPriority;
    uint32_t priority;
}

class GenerationIdOption extends HelloOption
{
    type = GenerationID;
    uint32_t generationID;
}

class AddressListOption extends HelloOption
{
    EncodedUnicastAddress secondaryAddresses[];
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Header
class PimPacket extends FieldsChunk
{
    short          version = 2;
    PimPacketType  type;
    short          reserved = 0;    // 8 bits
    uint16_t       crc = 0;         // The checksum is a standard IP checksum, i.e., the 16-bit one's
                                    // complement of the one's complement sum of the entire PIM
                                    // message, excluding the "Multicast data packet" section of the
                                    // Register message.  For computing the checksum, the checksum
                                    // field is zeroed.  If the packet's length is not an integral
                                    // number of 16-bit words, the packet is padded with a trailing
                                    // byte of zero before performing the checksum.
                                    //
                                    // For IPv6, the checksum also includes the IPv6 "pseudo-header",
                                    // as specified in RFC 2460, Section 8.1 [5].  This "pseudo-header"
                                    // is prepended to the PIM header for the purposes of calculating
                                    // the checksum.  The "Upper-Layer Packet Length" in the pseudo-
                                    // header is set to the length of the PIM message, except in
                                    // Register messages where it is set to the length of the PIM
                                    // register header (8).  The Next Header value used in the pseudo-
                                    // header is 103.
    CrcMode        crcMode = CRC_MODE_UNDEFINED;
}

// Hello message
// DM, SM
class PimHello extends PimPacket
{
    type = Hello;
    HelloOption *options[] @owned @allowReplace;
}

// Record for Join/Prune message
// in structure it is not possible to use dynamic arrays, it is needed to use class
class JoinPruneGroup extends cObject
{
    @packetData;
    EncodedGroupAddress   groupAddress;
    EncodedSourceAddress  joinedSourceAddress[];
    EncodedSourceAddress  prunedSourceAddress[];
}

// Join/Prune message
// DM, SM
class PimJoinPrune extends PimPacket
{
    type = JoinPrune;
    EncodedUnicastAddress  upstreamNeighborAddress;    //TODO replace to L3Address
    int                    reserved2 = 0;
    int                    holdTime;    // in seconds
    JoinPruneGroup         joinPruneGroups[];
}

// Assert message
// DM, SM
class PimAssert extends PimPacket
{
    type = Assert;
    EncodedGroupAddress    groupAddress;
    EncodedUnicastAddress  sourceAddress;
    bool                   R;
    int                    metric;
    int                    metricPreference;
}

// Graft message, only message send by unicast
// DM
class PimGraft extends PimJoinPrune
{
    type = Graft;
    holdTime = 0;
}

// GraftAck message
// DM
// create a PimGraft, then set type = GraftAck

// State Refresh message
// DM
class PimStateRefresh extends PimPacket
{
    type = StateRefresh;
    EncodedGroupAddress    groupAddress;
    EncodedUnicastAddress  sourceAddress;
    EncodedUnicastAddress  originatorAddress;
    bool                   R;
    int                    metricPreference;
    int                    metric;
    short                  maskLen;
    short                  ttl;
    bool                   P;
    bool                   N;
    bool                   O;
    short                  reserved2;
    short                  interval;
}
// Register message
// SM
class PimRegister extends PimPacket
{
    type = Register;
    bool            B;
    bool            N;
    uint32_t        reserved2;
}

// Register-Stop message
// SM
class PimRegisterStop extends PimPacket
{
    type = RegisterStop;
    EncodedGroupAddress    groupAddress;
    EncodedUnicastAddress  sourceAddress;
}
