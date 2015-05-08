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


cplusplus {{
#include "inet/networklayer/contract/ipv4/IPv4Address.h"

#define PIM_HEADER_LENGTH 4
#define ENCODED_UNICODE_ADDRESS_LENGTH 6 // IPv4 only
#define ENCODED_GROUP_ADDRESS_LENGTH   8 // IPv4 only
#define ENCODED_SOURCE_ADDRESS_LENGTH  8 // IPv4 only
}}

namespace inet;

//struct IPAddress;
class noncobject IPv4Address;

enum PIMPacketType
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

enum PIMHelloOptionType
{
   Holdtime = 1;
   LANPruneDelay = 2;
   DRPriority = 19;
   GenerationID = 20;
   StateRefreshCapable = 21;
   AddressList = 24;
}

class HelloOption
{
    short type enum(PIMHelloOptionType);
}

class HoldtimeOption extends HelloOption
{
    type = Holdtime;
    uint16_t holdTime;
}

class LANPruneDelayOption extends HelloOption
{
    type = LANPruneDelay;
    uint16_t propagationDelay;
    uint16_t overrideInterval;
}

class DRPriorityOption extends HelloOption
{
    type = DRPriority;
    uint32_t priority;
}

class GenerationIDOption extends HelloOption
{
    type = GenerationID;
    uint32_t generationID;
}

//class AddressListOption extends HelloOption
//{
//    IPv4Address secondaryAddresses[];
//}

cplusplus {{ class HelloOption; typedef HelloOption *HelloOptionPtr; }}
class noncobject HelloOptionPtr;

struct EncodedAddress
{
    IPv4Address IPaddress;
    bool         S;
    bool        W;                //WC bit
    bool        R;                //RPT bit
}

// Record for Join/Prune message
// in structure it is not possible to use dynamic arrays, iit is needed to use class
class JoinPruneGroup
{
    IPv4Address     groupAddress;
    EncodedAddress     joinedSourceAddress[];
    EncodedAddress     prunedSourceAddress[];
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Header
packet PIMPacket
{
   short         version = 2;
   short         type enum(PIMPacketType);
}

// Hello message
// DM, SM
packet PIMHello extends PIMPacket
{
    @customize(true);
    type = Hello;
    abstract HelloOptionPtr  options[];
}

// Join/Prune message
// DM, SM
packet PIMJoinPrune extends PIMPacket
{
    short             type enum(PIMPacketType) = JoinPrune;
    IPv4Address        upstreamNeighborAddress;
    int                holdTime;
    //int                numGroups;
    JoinPruneGroup    joinPruneGroups[];
}

// Assert message
// DM, SM
packet PIMAssert extends PIMPacket
{
    short             type enum(PIMPacketType) = Assert;
    IPv4Address        groupAddress;
    IPv4Address        sourceAddress;
    bool            R;
    int                metric;
    int                metricPreference;
}

// Graft message, only message send by unicast
// DM
packet PIMGraft extends PIMJoinPrune
{
    short             type enum(PIMPacketType) = Graft;
    int                holdTime = 0;
}

// GraftAck message
// DM
packet PIMGraftAck extends PIMGraft
{
    short             type enum(PIMPacketType) = GraftAck;
}

// State Refresh message
// DM
packet PIMStateRefresh extends PIMPacket
{
    short             type enum(PIMPacketType) = StateRefresh;
    IPv4Address        groupAddress;
    IPv4Address        sourceAddress;
    IPv4Address        originatorAddress;
    int                metric;
    int                metricPreference;
    short            maskLen;
    short            ttl;
    bool            P;
    int                interval;
}
// Register message
// SM
packet PIMRegister extends PIMPacket
{
    short             type enum(PIMPacketType) = Register;
    bool            B;
    bool            N;
}

// Register-Stop message
// SM
packet PIMRegisterStop extends PIMPacket
{
    short             type enum(PIMPacketType) = RegisterStop;
    IPv4Address        groupAddress;
    IPv4Address        sourceAddress;
}