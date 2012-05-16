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
#include "IPv4Address.h"
}}


class noncobject IPv4Address;

enum OSPFPacketType
{
    HELLO_PACKET = 1;
    DATABASE_DESCRIPTION_PACKET = 2;
    LINKSTATE_REQUEST_PACKET = 3;
    LINKSTATE_UPDATE_PACKET = 4;
    LINKSTATE_ACKNOWLEDGEMENT_PACKET = 5;
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
    char type enum(OSPFPacketType) = HELLO_PACKET;

    IPv4Address routerID;
    IPv4Address areaID;

    short authenticationType = 0;
    char authentication[8];
}

//
// Represents an OSPF Hello packet
//
packet OSPFHelloPacket extends OSPFPacket
{
    IPv4Address networkMask;

    short helloInterval = 5;

    OSPFOptions options;

    char routerPriority = 0;
    long routerDeadInterval = 0;

    IPv4Address designatedRouter;
    IPv4Address backupDesignatedRouter;
    IPv4Address neighbor[];
}


enum LSAType
{
    ROUTERLSA_TYPE = 1;
    NETWORKLSA_TYPE = 2;
    SUMMARYLSA_NETWORKS_TYPE = 3;
    SUMMARYLSA_ASBOUNDARYROUTERS_TYPE = 4;
    AS_EXTERNAL_LSA_TYPE = 5;
}

//
// Represents an OSPF LSA header
//
class OSPFLSAHeader
{
    unsigned short lsAge = 0;
    OSPFOptions lsOptions;
    char lsType enum(LSAType) = ROUTERLSA_TYPE;
    IPv4Address linkStateID;
    IPv4Address advertisingRouter;
    long lsSequenceNumber = 0;
    unsigned short lsaLength = 0;
}

//
// common ancestor type for all LSAs
//
class OSPFLSA
{
    OSPFLSAHeader header;
}

enum LinkType
{
    POINTTOPOINT_LINK = 1;
    TRANSIT_LINK = 2;
    STUB_LINK = 3;
    VIRTUAL_LINK = 4;
}

struct TOSData
{
    unsigned char tos;
    unsigned char tosMetric[3];
}

class Link
{
    IPv4Address linkID;
    unsigned long linkData = 0;
    unsigned char type enum(LinkType) = POINTTOPOINT_LINK;
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
    IPv4Address networkMask;
    IPv4Address attachedRouters[];
}

//
// Represents an OSPF Summary LSA
//
class OSPFSummaryLSA extends OSPFLSA
{
    IPv4Address networkMask;
    unsigned long routeCost = 1;
    TOSData tosData[];
}

struct ExternalTOSInfo
{
    TOSData tosData;
    bool E_ExternalMetricType;
    IPv4Address forwardingAddress;
    long externalRouteTag;
}

//
// Represents the contents of an OSPF AS External LSA
//
class OSPFASExternalLSAContents
{
    IPv4Address networkMask;
    bool E_ExternalMetricType = false;
    unsigned long routeCost = 1;
    IPv4Address forwardingAddress;
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
    IPv4Address linkStateID;
    IPv4Address advertisingRouter;
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
