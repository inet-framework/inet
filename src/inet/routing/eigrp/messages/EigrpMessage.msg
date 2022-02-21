//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// @author Vit Rek (rek@kn.vutbr.cz)
// @author Jan Bloudicek
// @author Jan Zavrel
// @author Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3

import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.networklayer.contract.ipv6.Ipv6Address;
import inet.common.packet.chunk.Chunk;

namespace inet;

cplusplus {{
using namespace inet;
}}

// Codes of EIGRP messages
enum HeaderOpcode
{
    EIGRP_UPDATE_MSG = 1;
    EIGRP_REQUEST_MSG = 2;
    EIGRP_QUERY_MSG = 3;
    EIGRP_REPLY_MSG = 4;
    EIGRP_HELLO_MSG = 5;
    EIGRP_SIAQUERY_MSG = 10;
    EIGRP_SIAREPLY_MSG = 11;
}

enum EigrpTlvTypeLow
{
    EIGRP_TLV_PARAM = 0;
    EIGRP_TLV_ROUTE = 2;
    EIGRP_TLV_STUB = 6;
}

// Struct for K-values
struct EigrpKValues
{
    @packetData;
    uint16_t K1 = 1;
    uint16_t K2 = 0;
    uint16_t K3 = 1;
    uint16_t K4 = 0;
    uint16_t K5 = 0;
    uint16_t K6 = 0;
}

// 8 bit flags field, only three flags specified
struct EigrpRouteFlags
{
    @packetData;
    bool SW;        // Source Withdraw
    bool CD;        // Candidate Default
    bool active;    // Active path
}

// Struct for wide metric paramaters
struct EigrpWideMetricPar
{
    @packetData;
    uint8_t offset = 0;
    uint8_t priority = 0;
    uint8_t reliability = 0;    // Min rel. on path
    uint8_t load = 0;           // Max load on path
    uint32_t mtu = 0;           // Min MTU on path, only 24 bits used
    uint8_t hopCount = 0;       // Hop count to destination
    uint64_t delay = 0;         // Delay sum on path, only 48 bits used
    uint64_t bandwidth = 0;     // Min BW on path, only 48 bits used
}

// Struct for EIGRP stub
struct EigrpStub
{
    @packetData;
    bool connectedRt = false;
    bool staticRt = false;
    bool summaryRt = false;
    bool redistributedRt = false;
    bool leakMapRt = false;
    bool recvOnlyRt = false;
}

// TLV Stub for Hello message
struct EigrpTlvStub
{
    @packetData;
    char typeHigh = 0;
    char typeLow = EIGRP_TLV_STUB;
    EigrpStub stub;
}

// TLV Parameter Type for Hello message
struct EigrpTlvParameter
{
    @packetData;
    // type
    char typeHigh = 0;
    char typeLow = EIGRP_TLV_PARAM;
    // value
    EigrpKValues kValues;
    uint16_t holdTimer;
};

// IPv4 TLV Internal route
//struct EigrpIpv4Internal
//{
//    // type
//    char typeHigh = 1;
//    char typeLow = 2;
//    // length
//    uint16_t length = 28;
//    // value
//    IPv4Address routerID;     // IP of originating router (also in exterior TLV)
//    IPv4Address nextHop;
//    EigrpMetricPar metric;
//    EigrpRouteFlags flags;
//    IPv4Address destMask;     // Destination
//    IPv4Address destAddress;
//};

// Multiprotocol Ipv4 (IPv4Address used) Internal TLV route
struct EigrpMpIpv4Internal
{
    @packetData;
    // type
    char typeHigh = 6;
    char typeLow = EIGRP_TLV_ROUTE;
    // value
    uint16_t afi;           // Address Family ID, IPv4 = 1, Ipv6 = 2
    uint16_t tid;           // Topology ID
    Ipv4Address routerID;   // Router ID

    EigrpWideMetricPar metric;
    EigrpRouteFlags flags;

    Ipv4Address nextHop;
    Ipv4Address destMask;
    Ipv4Address destAddress;
};

// Multiprotocol Ipv6 (IPv6Address used) Internal TLV route
struct EigrpMpIpv6Internal
{
    @packetData;
    // type
    char typeHigh = 6;
    char typeLow = EIGRP_TLV_ROUTE;
    // value
    uint16_t afi = 2;       // Address Family ID, IPv4 = 1, Ipv6 = 2
    uint16_t tid;           // Topology ID
    Ipv4Address routerID;   // Router ID

    EigrpWideMetricPar metric;
    EigrpRouteFlags flags;

    Ipv6Address nextHop;
    Ipv6Address destMask;
    Ipv6Address destAddress;
};

// General structure of EIGRP message with header
class EigrpMessage extends FieldsChunk
{
    chunkLength = B(4);
    char version = 2;   // Version of EIGRP header
    int8_t opcode;      // Type of message
    // flags
    bool init;          // Initialization - establishment of neighborship
    bool cr;            // Conditionally Received
    bool rs;            // Reset
    bool eot;           // End of table

    int seqNum;         // Sequence number
    int ackNum;         // Acknowledgement number
    uint16_t vrid;      // Virtual Router ID
    uint16_t asNum;     // Autonomous system number
};

// EIGRP Hello message

class EigrpIpv4Hello extends EigrpMessage
{
    EigrpTlvParameter parameterTlv;     // Always present
    EigrpTlvStub stubTlv;
};

class EigrpIpv6Hello extends EigrpMessage
{
    EigrpTlvParameter parameterTlv;     // Always present
    EigrpTlvStub stubTlv;
};

// Ack message - temporarily
class EigrpIpv4Ack extends EigrpMessage
{
};

// Ack message - temporarily
class EigrpIpv6Ack extends EigrpMessage
{
};

class EigrpIpv4Message extends EigrpMessage
{
    EigrpMpIpv4Internal interRoutes[]; //TODO - dost zvlastni pouziti, jestli bude cas tak predelat
};

class EigrpIpv6Message extends EigrpMessage
{
    EigrpMpIpv6Internal interRoutes[]; //TODO - dost zvlastni pouziti, jestli bude cas tak predelat - pouze kopiruji pristup z IPv4
};

// EIGRP Update Message
class EigrpIpv4Update extends EigrpIpv4Message
{
};

// EIGRP Update Message
class EigrpIpv6Update extends EigrpIpv6Message
{
};

// EIGRP Query Message
class EigrpIpv4Query extends EigrpIpv4Message
{
};

// EIGRP Query Message
class EigrpIpv6Query extends EigrpIpv6Message
{
};

// EIGRP Reply Message
class EigrpIpv4Reply extends EigrpIpv4Message
{
};

// EIGRP Reply Message
class EigrpIpv6Reply extends EigrpIpv6Message
{
};

//
// EIGRP RTP
//
struct EigrpMsgRoute
{
    int sourceId;           // ID of source for route
    int routeId;            // ID of route
    bool unreachable;       // Route with maximal metric
    bool invalid;
    Ipv4Address originator;
};

// Message request
message EigrpMsgReq
{
    @customize(true);
    int8_t opcode;              // Type of message
    bool goodbyeMsg = false;    // Message is Hello goodbye

    // flags
    bool init;                  // Initialization - establishment of neighborship
    bool cr;                    // Conditionally Received
    bool rs;                    // Reset
    bool eot;                   // End of table

    int destNeighbor;           // ID of neighbor that is destination of message
    int destInterface;          // ID of destination interface

    uint32_t seqNumber;         // Sequence number for reliable transmission
    uint32_t ackNumber;         // Ack number for reliable transmission

    int numOfAck;               // Number of acknowledges
    EigrpMsgRoute routes[];
};
