
import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.networklayer.contract.ipv6.Ipv6Address;
import inet.networklayer.common.L3Address;
import inet.routing.ospf.OspfPacket;


namespace inet;

//Common OSPFv3 packet header - this header is present in every OSPFv3 packet
class Ospfv3Packet extends ospf::OspfPacket
{
    chunkLength = B(16);
    version = 3;
    int8_t instanceID;
    uint8_t reserved = 0;

    // RFC 5340 A.3.1.  The OSPF Packet Header: Checksum:
    //   OSPF uses the standard checksum calculation for IPv6 applications:
    //   The 16-bit one's complement of the one's complement sum of the
    //   entire contents of the packet, starting with the OSPF packet
    //   header, and prepending a "pseudo-header" of IPv6 header fields, as
    //   specified in Section 8.1 of [IPV6].  The "Upper-Layer Packet
    //   Length" in the pseudo-header is set to the value of the OSPF
    //   packet header's length field.  The Next Header value used in the
    //   pseudo-header is 89.  If the packet's length is not an integral
    //   number of 16-bit words, the packet is padded with a byte of zero
    //   before checksumming.  Before computing the checksum, the checksum
    //   field in the OSPF packet header is set to 0.
}

//options for OSPFv3 packets - present in Hello, DD and certain LSAs
struct Ospfv3Options 
{
    bool reservedOne;	//reserved for compatibility with ospfv2
    bool reservedTwo;	//reserved for compatibility with ospfv2
    bool dcBit;			//demand circuits
    bool rBit;			//router bit indicating the router originating the message is active
    bool nBit;			//the router is attached to NSSA
    bool xBit = 0;		//used for MOSPF but is deprecated now and the bit should always be zero
    bool eBit;			//AS-extarnal flooding option
    bool v6Bit;			//0=router should be excluded from IPv6 routing calculations
}

//
//	OSPFv3 hello packet
//
class Ospfv3HelloPacket extends Ospfv3Packet
{
    type = ospf::HELLO_PACKET;
	uint32_t interfaceID;
	char routerPriority;
	
	Ospfv3Options options;
	
	unsigned short helloInterval;
	unsigned short deadInterval;
	
	Ipv4Address designatedRouterID;
	Ipv4Address backupDesignatedRouterID;
	Ipv4Address neighborID[];
}

//
//					LSAs
//
//struct OSPFv3LSAType
//{
//    bool uBit;	//controls action the router should take when it recieves unknown LSA Function Code
//    bool s1Bit;
//    bool s2Bit;
//    uint16_t lsaFunctionCode;
//}

//Common LSA Header - this header is present in every LSA
//triplet {lsType, linkStateID, advertisingRouter} uniquely identify the LSA
class Ospfv3LsaHeader extends cObject
{
    unsigned short lsaAge;
    unsigned short lsaType;
    
    Ipv4Address linkStateID;
    Ipv4Address advertisingRouter;
    
    uint32_t lsaSequenceNumber;
    
    unsigned short lsaChecksum;
    unsigned short lsaLength;
}

class Ospfv3Lsa extends cObject
{
    Ospfv3LsaHeader header;
}

//LSA Function Code
enum Ospfv3LsaFunctionCode
{
    ROUTER_LSA = 1;
    NETWORK_LSA = 2;
    INTER_AREA_PREFIX_LSA = 3;
    INTER_AREA_ROUTER_LSA = 4;
    AS_EXTERNAL_LSA = 5;
    DEPRECATED = 6;
    NSSA_LSA = 7;
    LINK_LSA = 8;
    INTRA_AREA_PREFIX_LSA = 9;
}

//used for Intra-Area-Prefix LSA
struct Ospfv3LsaPrefix 
{
    bool dnBit=0;
    bool pBit=0;
    bool xBit=0;
    bool laBit=0;
    bool nuBit=0;
    uint8_t prefixLen;
    
    //TODO address prefix
    L3Address addressPrefix;
    uint16_t metric; //for Intra-Area-Prefix LSA
}

//Address Prefix options
struct Ospfv3LinkLsaPrefix 
{
    bool dnBit;
    bool pBit;
    bool xBit;
    bool laBit;
    bool nuBit;
    uint16_t prefixLen;
    
    //TODO address prefix
    L3Address addressPrefix;
//    uint16_t metric; //for Intra-Area-Prefix LSA
}

//Router LSA
enum Ospfv3RouterLsaType
{
    POINT_TO_POINT = 1;
    TRANSIT_NETWORK = 2;
    RESERVED = 3;
    VIRTUAL_LINK = 4;
}

struct Ospfv3RouterLsaBody
{
    uint8_t type;
    uint16_t metric;
    
    uint32_t interfaceID;
    uint32_t neighborInterfaceID;
    Ipv4Address neighborRouterID;//TODO - check if this is a number of IP address
}

class Ospfv3RouterLsa extends Ospfv3Lsa
{
    bool ntBit;
    bool xBit;
    bool vBit;
    bool eBit;
    bool bBit;
    Ospfv3Options ospfOptions;
    
    Ospfv3RouterLsaBody routers[];
}

//Network LSA 
class Ospfv3NetworkLsa  extends Ospfv3Lsa
{
    Ospfv3Options ospfOptions;
    
    Ipv4Address attachedRouter[];
}

// Inter-Area-Prefix-LSAs 
class Ospfv3InterAreaPrefixLsa  extends Ospfv3Lsa
{
    bool dnBit;
    bool pBit;
    bool xBit;
    bool laBit;
    bool nuBit;
    uint32_t metric;
    uint8_t prefixLen;
    L3Address prefix;
}

//Inter-Area-Router-LSAs 
class Ospfv3InterAreaRouterLsa  extends Ospfv3Lsa
{
    Ospfv3Options ospfOptions;
    uint32_t metric;
    uint32_t destinationRouter;
}

//   AS-External-LSAs 
class Ospfv3AsExternalLsa  extends Ospfv3Lsa
{
    bool eBit;
    bool fBit;
    bool tBit;
    uint32_t metric;
    
    uint16_t referencedLSType;
    
    Ipv6Address forwardingAddress;
    uint32_t externalRouteTag;
    uint32_t referencedLSID; 
}

//   NSSA-LSAs  
// Routers in a Not-so-stubby-area (NSSA) do not receive external LSAs from Area Border Routers,
// but are allowed to send external routing information for redistribution.
// They use type 7 LSAs to tell the ABRs about these external routes,
// which the Area Border Router then translates to type 5 external LSAs and floods as normal to the rest of the OSPF network
class Ospfv3NssaLsa  extends Ospfv3Lsa
{//basically the same as the AS External LSA
    bool eBit;
    bool fBit;
    bool tBit;
    uint32_t metric;
    
    Ospfv3LsaPrefix prefOptions;
    uint16_t referencedLSType;
    
    Ipv6Address forwardingAddress;
    uint32_t externalRouteTag;
    uint32_t referencedLSID;
}

// Link-LSAs 
// A link-local only LSA for OSPFv3. A Type 8 LSA is used to give 
// information about link-local addresses and a list of IPv6 addresses on the link.
class Ospfv3LinkLsa  extends Ospfv3Lsa
{
    uint8_t routerPriority;
    Ospfv3Options ospfOptions;
    
    L3Address linkLocalInterfaceAdd;
    
    uint32_t numPrefixes;
    Ospfv3LinkLsaPrefix prefixes[];
}

// Intra-Area-Prefix-LSAs 
class Ospfv3IntraAreaPrefixLsa  extends Ospfv3Lsa
{
    unsigned short numPrefixes;
    unsigned short referencedLSType;
    Ipv4Address referencedLSID;
    Ipv4Address referencedAdvRtr;
    
    Ospfv3LsaPrefix prefixes[];
}

//
//			Database Description Packet
//


//options struct for Database Description packets
struct Ospfv3DdOptions{
    bool iBit;	//this packet is the initial DD packet
    bool mBit;	//more DD packets will follow
    bool msBit;	//1=the router is the master, 0=router is the slave
}

class Ospfv3DatabaseDescriptionPacket extends Ospfv3Packet
{
    type = ospf::DATABASE_DESCRIPTION_PACKET;
    uint8_t reserved1 = 0;
    Ospfv3Options options;
    uint16_t interfaceMTU;
    uint16_t reserved2 = 0;     // 13 bits
    Ospfv3DdOptions ddOptions;

    uint32_t sequenceNumber;

    Ospfv3LsaHeader lsaHeaders[];
}


//
//				Link State Request				  
//

//identity of requested LSA - there may be multiple LSAs requested in a single LSR
struct Ospfv3LsRequest{
    uint16_t lsaType;
    Ipv4Address lsaID;
    Ipv4Address advertisingRouter;
}

//Link State Request packet
class Ospfv3LinkStateRequestPacket extends Ospfv3Packet
{
    Ospfv3LsRequest requests[];
}

//
//			Link State Update				 
//

//Link State Update packet
class Ospfv3LsUpdate extends Ospfv3Packet{
    uint32_t lsaCount;	//specifies the number of LSAs sent in a single packet
    
    Ospfv3RouterLsa RouterLSAs[];
    Ospfv3NetworkLsa NetworkLSAs[];
    Ospfv3InterAreaPrefixLsa InterAreaPrefixLSAs[];
    Ospfv3LinkLsa LinkLSAs[];
    Ospfv3IntraAreaPrefixLsa IntraAreaPrefixLSAs[];
    
//    Ospfv3Lsa LSAs[];
    //TODO - template for any LSA??
}

//
//			Link State Acknowledgement
//

//Link State Acknowledgement packet
class Ospfv3LinkStateAcknowledgementPacket extends Ospfv3Packet
{
    uint32_t lsas;
    Ospfv3LsaHeader lsaHeaders[];
}

