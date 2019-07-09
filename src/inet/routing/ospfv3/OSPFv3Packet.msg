
import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.networklayer.contract.ipv6.Ipv6Address;
import inet.networklayer.common.L3Address;


namespace inet;
enum OSPFv3PacketType
{
	HELLO_PACKET = 1;
	DATABASE_DESCRIPTION = 2;
	LSR = 3;
	LSU = 4;
	LS_ACK = 5;
}

//Common OSPFv3 packet header - this header is present in every OSPFv3 packet
class OSPFv3Packet extends FieldsChunk
{
	char version = 3;
	char type;
	uint16_t packetLength;
	
    Ipv4Address routerID;
    Ipv4Address areaID;
    
    uint16_t checksum;
    char instanceID;
}

//options for OSPFv3 packets - present in Hello, DD and certain LSAs
struct OSPFv3Options 
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
class OSPFv3HelloPacket extends OSPFv3Packet 
{
	uint32_t interfaceID;
	char routerPriority;
	
	OSPFv3Options options;
	
	uint16_t helloInterval;
	uint16_t deadInterval;
	
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
class OSPFv3LSAHeader extends cObject
{
    uint16_t lsaAge;
    uint16_t lsaType;
    
    Ipv4Address linkStateID;
    Ipv4Address advertisingRouter;
    
    uint32_t lsaSequenceNumber;
    
    uint16_t lsaChecksum;
    uint16_t lsaLength;
}

class OSPFv3LSA extends cObject
{
    OSPFv3LSAHeader header;
}

//LSA Function Code
enum OSPFv3LSAFunctionCode
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
struct OSPFv3LSAPrefix 
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
struct OSPFv3LinkLSAPrefix 
{
    bool dnBit;
    bool pBit;
    bool xBit;
    bool laBit;
    bool nuBit;
    uint8_t prefixLen;
    
    //TODO address prefix
    L3Address addressPrefix;
//    uint16_t metric; //for Intra-Area-Prefix LSA
}

//Router LSA
enum OSPFv3RouterLSAType
{
    POINT_TO_POINT = 1;
    TRANSIT_NETWORK = 2;
    RESERVED = 3;
    VIRTUAL_LINK = 4;
}

struct OSPFv3RouterLSABody
{
    uint8_t type;
    uint16_t metric;
    
    uint32_t interfaceID;
    uint32_t neighborInterfaceID;
    Ipv4Address neighborRouterID;//TODO - check if this is a number of IP address
}

class OSPFv3RouterLSA extends OSPFv3LSA
{
    bool ntBit;
    bool xBit;
    bool vBit;
    bool eBit;
    bool bBit;
    OSPFv3Options ospfOptions;
    
    OSPFv3RouterLSABody routers[];
}

//Network LSA 
class OSPFv3NetworkLSA  extends OSPFv3LSA
{
    OSPFv3Options ospfOptions;
    
    Ipv4Address attachedRouter[];
}

// Inter-Area-Prefix-LSAs 
class OSPFv3InterAreaPrefixLSA  extends OSPFv3LSA
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
class OSPFv3InterAreaRouterLSA  extends OSPFv3LSA
{
    OSPFv3Options ospfOptions;
    uint32_t metric;
    uint32_t destinationRouter;
}

//   AS-External-LSAs 
class OSPFv3ASExternalLSA  extends OSPFv3LSA
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
class OSPFv3NssaLSA  extends OSPFv3LSA
{//basically the same as the AS External LSA
    bool eBit;
    bool fBit;
    bool tBit;
    uint32_t metric;
    
    OSPFv3LSAPrefix prefOptions;
    uint16_t referencedLSType;
    
    Ipv6Address forwardingAddress;
    uint32_t externalRouteTag;
    uint32_t referencedLSID;
}

// Link-LSAs 
// A link-local only LSA for OSPFv3. A Type 8 LSA is used to give 
// information about link-local addresses and a list of IPv6 addresses on the link.
class OSPFv3LinkLSA  extends OSPFv3LSA
{
    uint8_t routerPriority;
    OSPFv3Options ospfOptions;
    
    L3Address linkLocalInterfaceAdd;
    
    uint32_t numPrefixes;
    OSPFv3LinkLSAPrefix prefixes[];
}

// Intra-Area-Prefix-LSAs 
class OSPFv3IntraAreaPrefixLSA  extends OSPFv3LSA
{
    uint16_t numPrefixes;
    uint16_t referencedLSType;
    Ipv4Address referencedLSID;
    Ipv4Address referencedAdvRtr;
    
    OSPFv3LSAPrefix prefixes[];
}

//
//			Database Description Packet
//


//options struct for Database Description packets
struct OSPFv3DDOptions{
    bool iBit;	//this packet is the initial DD packet
    bool mBit;	//more DD packets will follow
    bool msBit;	//1=the router is the master, 0=router is the slave
}

class OSPFv3DatabaseDescription extends OSPFv3Packet {
    OSPFv3Options options;
    
    uint16_t interfaceMTU;
    OSPFv3DDOptions ddOptions;
    
    uint32_t sequenceNumber;
    
    OSPFv3LSAHeader lsaHeaders[];
}


//
//				Link State Request				  
//

//identity of requested LSA - there may be multiple LSAs requested in a single LSR
struct OSPFv3LSRequest{
    uint16_t lsaType;
    Ipv4Address lsaID;
    Ipv4Address advertisingRouter;
}

//Link State Request packet
class OSPFv3LinkStateRequest extends OSPFv3Packet{
    OSPFv3LSRequest requests[];
}

//
//			Link State Update				 
//

//Link State Update packet
class OSPFv3LSUpdate extends OSPFv3Packet{
    uint32_t lsaCount;	//specifies the number of LSAs sent in a single packet
    
    OSPFv3RouterLSA RouterLSAs[];
    OSPFv3NetworkLSA NetworkLSAs[];
    OSPFv3InterAreaPrefixLSA InterAreaPrefixLSAs[];
    OSPFv3LinkLSA LinkLSAs[];
    OSPFv3IntraAreaPrefixLSA IntraAreaPrefixLSAs[];
    
//    OSPFv3LSA LSAs[];
    //TODO - template for any LSA??
}

//
//			Link State Acknowledgement
//

//Link State Acknowledgement packet
class OSPFv3LSAck extends OSPFv3Packet{
    uint32_t lsas;
    OSPFv3LSAHeader lsaHeaders[];
}