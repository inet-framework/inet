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

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.transportlayer.common.CrcMode;

namespace inet::ospf;

enum OspfPacketType
{
    HELLO_PACKET = 1;
    DATABASE_DESCRIPTION_PACKET = 2;
    LINKSTATE_REQUEST_PACKET = 3;
    LINKSTATE_UPDATE_PACKET = 4;
    LINKSTATE_ACKNOWLEDGEMENT_PACKET = 5;
}

// should be a byte long bitfield
struct OspfOptions
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
class OspfPacket extends FieldsChunk
{
    char version = 2;
    OspfPacketType type = static_cast<OspfPacketType>(-1);

    Ipv4Address routerID;
    Ipv4Address areaID;

    uint16_t crc = 0;
    CrcMode crcMode = CRC_MODE_UNDEFINED;

    short authenticationType = 0;
    char authentication[8];
}

//
// Represents an OSPF Hello packet
//
class OspfHelloPacket extends OspfPacket
{
    type = HELLO_PACKET;
    Ipv4Address networkMask;

    short helloInterval = 5;

    OspfOptions options;

    char routerPriority = 0;
    long routerDeadInterval = 0;

    Ipv4Address designatedRouter;
    Ipv4Address backupDesignatedRouter;
    Ipv4Address neighbor[];
}


enum LsaType
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
class OspfLsaHeader extends cObject
{
    unsigned short lsAge = 0;
    OspfOptions lsOptions;
    LsaType lsType = static_cast<LsaType>(-1);
    Ipv4Address linkStateID;
    Ipv4Address advertisingRouter;
    int32_t lsSequenceNumber = 0;
    uint16_t lsCrc = 0;
    CrcMode lsCrcMode = CRC_MODE_UNDEFINED;
    uint16_t lsaLength = 0;
}

//
// common ancestor type for all LSAs
//
class OspfLsa extends cObject
{
    OspfLsaHeader header;
}

enum LinkType
{
    POINTTOPOINT_LINK = 1;
    TRANSIT_LINK = 2;
    STUB_LINK = 3;
    VIRTUAL_LINK = 4;
}

// (RFC 1583 Section A.4.2.)
struct TosData
{
    unsigned char tos;
    unsigned long tosMetric;
}

// Router LSA Link section (RFC 1583 Section A.4.2.)
class Link extends cObject
{
    Ipv4Address linkID;
    unsigned long linkData = 0;
    LinkType type = POINTTOPOINT_LINK;
    unsigned char numberOfTOS = 0;
    unsigned long linkCost = 1;     // TOS 0 metric
    TosData tosData[];
}

//
// Represents an OSPF Router LSA (RFC 1583 Section A.4.2.)
//
class OspfRouterLsa extends OspfLsa
{
    // header.lsType = ROUTERLSA_TYPE;
    unsigned short reserved1 = 0;   // 5 bit
    bool V_VirtualLinkEndpoint = false;
    bool E_ASBoundaryRouter = false;
    bool B_AreaBorderRouter = false;
    unsigned short reserved2 = 0;   // 8 bit
    unsigned short numberOfLinks = 0;
    Link links[];
}

//
// Represents an OSPF Network LSA
//
class OspfNetworkLsa extends OspfLsa
{
    // header.lsType = NETWORKLSA_TYPE;
    Ipv4Address networkMask;
    Ipv4Address attachedRouters[];
}

//
// Represents an OSPF Summary LSA
//
class OspfSummaryLsa extends OspfLsa
{
    // header.lsType = SUMMARYLSA_NETWORKS_TYPE;
    Ipv4Address networkMask;
    unsigned long routeCost = 1;
    TosData tosData[];  // 1 element required, count of tosData not stored, calculate it from length of LSA
}

struct ExternalTosInfo
{
    // TODO: tosData (?)
    unsigned short tos;
    bool E_ExternalMetricType = false;
    unsigned long routeCost = 1;
    Ipv4Address forwardingAddress;
    long externalRouteTag;
}

//
// Represents the contents of an OSPF AS External LSA
//
class OspfAsExternalLsaContents extends cObject
{
    Ipv4Address networkMask;
    ExternalTosInfo externalTOSInfo[];
}

//
// Represents an OSPF AS External LSA
//
class OspfAsExternalLsa extends OspfLsa
{
    // header.lsType = AS_EXTERNAL_LSA_TYPE;
    OspfAsExternalLsaContents contents;
}


// should be a byte long bitfield
struct OspfDdOptions
{
    unsigned short unused = 0;  // 5 bit
    bool I_Init;
    bool M_More;
    bool MS_MasterSlave;
}

//
// Represents an OSPF Database Description packet
//
class OspfDatabaseDescriptionPacket extends OspfPacket
{
    unsigned short interfaceMTU;
    OspfOptions options;
    OspfDdOptions ddOptions;
    unsigned long ddSequenceNumber;
    OspfLsaHeader lsaHeaders[];     //TODO
}

struct LsaRequest
{
    unsigned long lsType;
    Ipv4Address linkStateID;
    Ipv4Address advertisingRouter;
}

//
// Represents an OSPF Link State Request packet
//
class OspfLinkStateRequestPacket extends OspfPacket
{
    LsaRequest requests[];
}

//
// Represents an OSPF Link State Update packet
//
class OspfLinkStateUpdatePacket extends OspfPacket
{
    OspfLsa *ospfLSAs[] @owned @allowReplace;
}

//
// Represents an OSPF Link State Acknowledgement packet
//
class OspfLinkStateAcknowledgementPacket extends OspfPacket
{
    OspfLsaHeader lsaHeaders[];     //TODO
}

