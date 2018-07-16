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
// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)


import inet.common.INETDefs;
import inet.common.Units;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.ipv4.Ipv4Address;

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
   StateRefresh = 9;
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

class HelloOption extends cObject
{
    PimHelloOptionType type;
}

class HoldtimeOption extends HelloOption
{
    type = Holdtime;
    uint16_t holdTime;
}

class LanPruneDelayOption extends HelloOption
{
    type = LANPruneDelay;
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

//class AddressListOption extends HelloOption
//{
//    Ipv4Address secondaryAddresses[];
//}

struct EncodedAddress
{
    Ipv4Address IPaddress;
    bool        S;
    bool        W;                //WC bit
    bool        R;                //RPT bit
}

// Record for Join/Prune message
// in structure it is not possible to use dynamic arrays, iit is needed to use class
class JoinPruneGroup extends cObject
{
    Ipv4Address     groupAddress;
    EncodedAddress  joinedSourceAddress[];
    EncodedAddress  prunedSourceAddress[];
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Header
class PimPacket extends FieldsChunk
{
    short         version = 2;
    PimPacketType type;
}

// Hello message
// DM, SM
class PimHello extends PimPacket
{
    type = Hello;
    HelloOption *options[] @owned @allowReplace;
}

// Join/Prune message
// DM, SM
class PimJoinPrune extends PimPacket
{
    type= JoinPrune;
    Ipv4Address        upstreamNeighborAddress;
    int                holdTime;
    //int                numGroups;
    JoinPruneGroup    joinPruneGroups[];
}

// Assert message
// DM, SM
class PimAssert extends PimPacket
{
    type = Assert;
    Ipv4Address        groupAddress;
    Ipv4Address        sourceAddress;
    bool               R;
    int                metric;
    int                metricPreference;
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
    Ipv4Address        groupAddress;
    Ipv4Address        sourceAddress;
    Ipv4Address        originatorAddress;
    int                metric;
    int                metricPreference;
    short              maskLen;
    short              ttl;
    bool               P;
    int                interval;
}
// Register message
// SM
class PimRegister extends PimPacket
{
    type = Register;
    bool            B;
    bool            N;
}

// Register-Stop message
// SM
class PimRegisterStop extends PimPacket
{
    type = RegisterStop;
    Ipv4Address        groupAddress;
    Ipv4Address        sourceAddress;
}

