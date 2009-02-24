//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
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

cplusplus {{
#include "IPAddress.h"
}}


class noncobject IPAddress;

enum OSPFPacketType
{

    HelloPacket = 1;
    DatabaseDescriptionPacket = 2;
    LinkStateRequestPacket = 3;
    LinkStateUpdatePacket = 4;
    LinkStateAcknowledgementPacket = 5;
}

// should be a byte long bitfield
struct OSPFOptions
{
    bool unused_1;
    bool E_ExternalRoutingCapability;
    bool MC_MulticastForwarding;
    bool NP_Type7LSA;
    bool EA_ForwardExternalLSAs;
    bool DC_DemandCircuits;
    bool unused_2;
    bool unused_3;
}

//
// Represents an OSPF packet header
//
packet OSPFPacket
{
    char version = 2;
    char type enum(OSPFPacketType) = HelloPacket;
    short packetLength = 0;

    IPAddress routerID;
    IPAddress areaID;

    short checksum = 0;
    short authenticationType = 0;
    char authentication[8];
}

//
// Represents an OSPF Hello packet
//
packet OSPFHelloPacket extends OSPFPacket
{
    IPAddress networkMask;

    short helloInterval = 5;

    OSPFOptions options;

    char routerPriority = 0;
    long routerDeadInterval = 0;

    IPAddress designatedRouter;
    IPAddress backupDesignatedRouter;
    IPAddress neighbor[];
}


enum LSAType
{

    RouterLSAType = 1;
    NetworkLSAType = 2;
    SummaryLSA_NetworksType = 3;
    SummaryLSA_ASBoundaryRoutersType = 4;
    ASExternalLSAType = 5;
}

//
// Represents an OSPF LSA header
//
//class OSPFLSAHeader extends cObject
class OSPFLSAHeader
{
    unsigned short lsAge = 0;
    OSPFOptions lsOptions;
    char lsType enum(LSAType) = RouterLSAType;
    unsigned long linkStateID;
    IPAddress advertisingRouter;
    long lsSequenceNumber = 0;
    short lsChecksum = 0;
    unsigned short lsaLength = 0;
}

//
// common ancestor type for all LSAs
//
//class OSPFLSA extends cObject
class OSPFLSA
{
    OSPFLSAHeader header;
}

enum LinkType
{

    PointToPointLink = 1;
    TransitLink = 2;
    StubLink = 3;
    VirtualLink = 4;
}

struct TOSData
{
    unsigned char tos;
    unsigned char tosMetric[3];
}

//class Link extends cObject
class Link
{
    IPAddress linkID;
    unsigned long linkData = 0;
    unsigned char type enum(LinkType) = PointToPointLink;
    unsigned char numberOfTOS = 0;
    unsigned long linkCost = 1;
    TOSData tosData[];
}

//
// Represents an OSPF Router LSA
//
class OSPFRouterLSA extends OSPFLSA
{
    bool V_VirtualLinkEndpoint = false;
    bool E_ASBoundaryRouter = false;
    bool B_AreaBorderRouter = false;
    unsigned short numberOfLinks = 0;
    Link links[];
}

//
// Represents an OSPF Network LSA
//
class OSPFNetworkLSA extends OSPFLSA
{
    IPAddress networkMask;
    IPAddress attachedRouters[];
}

//
// Represents an OSPF Summary LSA
//
class OSPFSummaryLSA extends OSPFLSA
{
    IPAddress networkMask;
    unsigned long routeCost = 1;
    TOSData tosData[];
}

struct ExternalTOSInfo
{
    TOSData tosData;
    bool E_ExternalMetricType;
    IPAddress forwardingAddress;
    long externalRouteTag;
}

//
// Represents the contents of an OSPF AS External LSA
//
class OSPFASExternalLSAContents
{
    IPAddress networkMask;
    bool E_ExternalMetricType = false;
    unsigned long routeCost = 1;
    IPAddress forwardingAddress;
    long externalRouteTag = 0;
    ExternalTOSInfo externalTOSInfo[];
}

//
// Represents an OSPF AS External LSA
//
class OSPFASExternalLSA extends OSPFLSA
{
    OSPFASExternalLSAContents contents;
}


// should be a byte long bitfield
struct OSPFDDOptions
{
    bool unused_1;
    bool unused_2;
    bool unused_3;
    bool unused_4;
    bool unused_5;
    bool I_Init;
    bool M_More;
    bool MS_MasterSlave;
}

//
// Represents an OSPF Database Description packet
//
packet OSPFDatabaseDescriptionPacket extends OSPFPacket
{
    unsigned short interfaceMTU;
    OSPFOptions options;
    OSPFDDOptions ddOptions;
    unsigned long ddSequenceNumber;
    OSPFLSAHeader lsaHeaders[];
}

struct LSARequest
{
    unsigned long lsType;
    unsigned long linkStateID;
    IPAddress advertisingRouter;
}

//
// Represents an OSPF Link State Request packet
//
packet OSPFLinkStateRequestPacket extends OSPFPacket
{
    LSARequest requests[];
}

//
// Represents an OSPF Link State Update packet
//
packet OSPFLinkStateUpdatePacket extends OSPFPacket
{
    unsigned long numberOfLSAs;
    OSPFRouterLSA routerLSAs[];
    OSPFNetworkLSA networkLSAs[];
    OSPFSummaryLSA summaryLSAs[];
    OSPFASExternalLSA asExternalLSAs[];
}

//
// Represents an OSPF Link State Acknowledgement packet
//
packet OSPFLinkStateAcknowledgementPacket extends OSPFPacket
{
    OSPFLSAHeader lsaHeaders[];
}
