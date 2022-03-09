//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.routing.ospf_common.OspfPacketBase;
import inet.transportlayer.common.CrcMode;

namespace inet::ospfv2;

cplusplus {{
using namespace ospf;
}}

enum Ospfv2TimerType {
    INTERFACE_HELLO_TIMER = 0;
    INTERFACE_WAIT_TIMER = 1;
    INTERFACE_ACKNOWLEDGEMENT_TIMER = 3;
    NEIGHBOR_INACTIVITY_TIMER = 4;
    NEIGHBOR_POLL_TIMER = 5;
    NEIGHBOR_DD_RETRANSMISSION_TIMER = 6;
    NEIGHBOR_UPDATE_RETRANSMISSION_TIMER = 7;
    NEIGHBOR_REQUEST_RETRANSMISSION_TIMER = 8;
    DATABASE_AGE_TIMER = 9;
};

// should be a byte long bitfield
struct Ospfv2Options
{
    @packetData;
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
class Ospfv2Packet extends ospf::OspfPacketBase
{
    chunkLength = B(24);
    version = 2;

    short authenticationType = 0;   // 2 bytes
    char authentication[8];         // 8 bytes
}

//
// Represents an OSPF Hello packet
//
class Ospfv2HelloPacket extends Ospfv2Packet
{
    type = HELLO_PACKET;
    Ipv4Address networkMask;

    short helloInterval = 5;

    Ospfv2Options options;

    char routerPriority = 0;
    long routerDeadInterval = 0;

    Ipv4Address designatedRouter;
    Ipv4Address backupDesignatedRouter;
    Ipv4Address neighbor[];
}

enum Ospfv2LsaType
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
class Ospfv2LsaHeader extends cObject
{
    @packetData;
    unsigned short lsAge = 0;
    Ospfv2Options lsOptions;
    Ospfv2LsaType lsType = static_cast<Ospfv2LsaType>(-1);
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
class Ospfv2Lsa extends cObject
{
    @packetData;
    Ospfv2LsaHeader header;
}

enum LinkType
{
    POINTTOPOINT_LINK = 1;
    TRANSIT_LINK = 2;
    STUB_LINK = 3;
    VIRTUAL_LINK = 4;
}

// (RFC 1583 Section A.4.2.)
struct Ospfv2TosData
{
    @packetData;
    unsigned char tos;
    unsigned long tosMetric;
}

// Router LSA Link section (RFC 1583 Section A.4.2.)
class Ospfv2Link extends cObject
{
    @packetData;
    Ipv4Address linkID;
    unsigned long linkData = 0;
    LinkType type = POINTTOPOINT_LINK;
    unsigned char numberOfTOS = 0;
    unsigned long linkCost = 1;     // TOS 0 metric
    Ospfv2TosData tosData[];
}

//
// Represents an OSPF Router LSA (RFC 1583 Section A.4.2.)
//
class Ospfv2RouterLsa extends Ospfv2Lsa
{
    // header.lsType = ROUTERLSA_TYPE;
    unsigned short reserved1 = 0;   // 5 bit
    bool V_VirtualLinkEndpoint = false;
    bool E_ASBoundaryRouter = false;
    bool B_AreaBorderRouter = false;
    unsigned short reserved2 = 0;   // 8 bit
    unsigned short numberOfLinks = 0;
    Ospfv2Link links[];
}

//
// Represents an OSPF Network LSA
//
class Ospfv2NetworkLsa extends Ospfv2Lsa
{
    // header.lsType = NETWORKLSA_TYPE;
    Ipv4Address networkMask;
    Ipv4Address attachedRouters[];
}

//
// Represents an OSPF Summary LSA
//
class Ospfv2SummaryLsa extends Ospfv2Lsa
{
    // header.lsType = SUMMARYLSA_NETWORKS_TYPE;
    Ipv4Address networkMask;
    unsigned long routeCost = 1;
    Ospfv2TosData tosData[];  // 1 element required, count of tosData not stored, calculate it from length of LSA
}

struct Ospfv2ExternalTosInfo
{
    @packetData;
    // TODO tosData (?)
    unsigned short tos;
    bool E_ExternalMetricType = false;
    unsigned long routeCost = 1;
    Ipv4Address forwardingAddress;
    long externalRouteTag;
}

//
// Represents the contents of an OSPF AS External LSA
//
class Ospfv2AsExternalLsaContents extends cObject
{
    @packetData;
    Ipv4Address networkMask;
    Ospfv2ExternalTosInfo externalTOSInfo[];
}

//
// Represents an OSPF AS External LSA
//
class Ospfv2AsExternalLsa extends Ospfv2Lsa
{
    // header.lsType = AS_EXTERNAL_LSA_TYPE;
    Ospfv2AsExternalLsaContents contents;
}

// should be a byte long bitfield
struct Ospfv2DdOptions
{
    @packetData;
    unsigned short unused = 0;  // 5 bit
    bool I_Init;
    bool M_More;
    bool MS_MasterSlave;
}

//
// Represents an OSPF Database Description packet
//
class Ospfv2DatabaseDescriptionPacket extends Ospfv2Packet
{
    unsigned short interfaceMTU;
    Ospfv2Options options;
    Ospfv2DdOptions ddOptions;
    unsigned long ddSequenceNumber;
    Ospfv2LsaHeader lsaHeaders[];     //TODO
}

struct Ospfv2LsaRequest
{
    @packetData;
    unsigned long lsType;
    Ipv4Address linkStateID;
    Ipv4Address advertisingRouter;
}

//
// Represents an OSPF Link State Request packet
//
class Ospfv2LinkStateRequestPacket extends Ospfv2Packet
{
    Ospfv2LsaRequest requests[];
}

//
// Represents an OSPF Link State Update packet
//
class Ospfv2LinkStateUpdatePacket extends Ospfv2Packet
{
    Ospfv2Lsa *ospfLSAs[] @owned @allowReplace;
}

//
// Represents an OSPF Link State Acknowledgement packet
//
class Ospfv2LinkStateAcknowledgementPacket extends Ospfv2Packet
{
    Ospfv2LsaHeader lsaHeaders[];     //TODO
}
