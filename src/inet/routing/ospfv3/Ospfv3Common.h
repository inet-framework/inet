#ifndef __INET_OSPFV3COMMON_H_
#define __INET_OSPFV3COMMON_H_

#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/routing/ospfv3/Ospfv3Packet_m.h"

namespace inet {
namespace ospfv3 {

// intervals values
#define OSPFV3_START            5
#define METRIC                  10
#define DEFAULT_HELLO_INTERVAL  10
#define DEFAULT_DEAD_INTERVAL   40
#define MIN_LS_INTERVAL         5

#define DEFAULT_IPV4_INSTANCE   64
#define DEFAULT_IPV6_INSTANCE   0
#define DEFAULT_ROUTER_PRIORITY 1
#define IPV4INSTANCE            1
#define IPV6INSTANCE            2
#define VIRTUAL_LINK_TTL        32
#define MAX_AGE                 3600
#define MAX_AGE_DIFF            900
#define MIN_LS_ARRIVAL          1
#define INITIAL_SEQUENCE_NUMBER -2147483647
#define MAX_SEQUENCE_NUMBER     2147483647
#define MAX_SPF_WAIT_TIME       10000
#define MIN_SPF_WAIT_TIME       10000
#define REFERENCE_BANDWIDTH     100

#define LS_REFRESH_TIME         1800
#define CHECK_AGE               300
#define LS_INFINITY             16777215

//#define IPV6_DATAGRAM_LENGTH    65535
//#define OSPFV3_HEADER_LENGTH    16
//#define OSPFV3_DD_HEADER_LENGTH 12
//#define OSPFV3_LSA_HEADER_LENGTH  20
//#define OSPFV3_LSR_LENGTH       12
//#define OSPFV3_ROUTER_LSA_HEADER_LENGTH 4
//#define OSPFV3_ROUTER_LSA_BODY_LENGTH 16
//
//#define OSPFV3_NETWORK_LSA_HEADER_LENGTH 4
//#define OSPFV3_NETWORK_LSA_ATTACHED_LENGTH 4
//
//#define OSPFV3_INTER_AREA_PREFIX_LSA_HEADER_LENGTH 4
//#define OSPFV3_INTER_AREA_PREFIX_LSA_BODY_LENGTH 24
//
//#define OSPFV3_INTRA_AREA_PREFIX_LSA_HEADER_LENGTH 12
//#define OSPFV3_INTRA_AREA_PREFIX_LSA_PREFIX_LENGTH 20
//
//#define OSPFV3_LINK_LSA_BODY_LENGTH 24
//#define OSPFV3_LINK_LSA_PREFIX_LENGTH 20 //this is just temporary - the prefix now has fixed size

const B IPV6_DATAGRAM_LENGTH                        = B(65535);
const B OSPFV3_HEADER_LENGTH                        = B(16);
const B OSPFV3_DD_HEADER_LENGTH                     = B(12);
const B OSPFV3_LSA_HEADER_LENGTH                    = B(20);
const B OSPFV3_LSR_LENGTH                           = B(12);
const B OSPFV3_ROUTER_LSA_HEADER_LENGTH             = B(4);
const B OSPFV3_ROUTER_LSA_BODY_LENGTH               = B(16);

const B OSPFV3_NETWORK_LSA_HEADER_LENGTH            = B(4);
const B OSPFV3_NETWORK_LSA_ATTACHED_LENGTH          = B(4);

const B OSPFV3_INTER_AREA_PREFIX_LSA_HEADER_LENGTH  = B(4);
const B OSPFV3_INTER_AREA_PREFIX_LSA_BODY_LENGTH    = B(24);


const B OSPFV3_INTRA_AREA_PREFIX_LSA_HEADER_LENGTH  = B(12);
const B OSPFV3_INTRA_AREA_PREFIX_LSA_PREFIX_LENGTH  = B(20);

const B OSPFV3_LINK_LSA_BODY_LENGTH                 = B(24);
const B OSPFV3_LINK_LSA_PREFIX_LENGTH               = B(20);  //this is just temporary - the prefix now has fixed size
const B OSPFV3_LSA_PREFIX_HEADER_LENGTH        = B(4); //fixed part of prefix - PrefixLength (1B), PrefixOptions (1B), "0" (2B)

#define ROUTER_LSA_FUNCTION_CODE 1

const Ipv4Address NULL_ROUTERID(0, 0, 0, 0);
const Ipv4Address BACKBONE_AREAID(0, 0, 0, 0);
const Ipv4Address NULL_LINKSTATEID(0, 0, 0, 0);
const Ipv4Address NULL_IPV4ADDRESS(0, 0, 0, 0);


typedef Ipv4Address AreaID;
typedef unsigned int Metric;


struct Ipv6AddressRange
{
    Ipv6Address prefix;
    short prefixLength;
    Ipv6AddressRange() : prefix(), prefixLength() {}
    Ipv6AddressRange(Ipv6Address prefixPar, int prefixLengthPar) : prefix(prefixPar), prefixLength(prefixLengthPar) {}

    bool operator<(const Ipv6AddressRange& other) const
    {
        return (prefixLength > other.prefixLength) || ((prefixLength == other.prefixLength) && (prefix < other.prefix));
    }

    bool operator==(const Ipv6AddressRange& other) const
    {
        return (prefix == other.prefix) && (prefixLength == other.prefixLength);
    }

    bool contains(const Ipv6Address& other) const // ma to robit to, ze vezme other a zisti, ci patri pod adresu siete ,respektive ci maju prefix a other rovnaku siet (cize prefix s dlzkou prefixlen musi byt rovnaky ako other s dlzkou prefixlen)
    {
        return (prefix.getPrefix(prefixLength) == other.getPrefix(prefixLength));
    }

    bool contains(const Ipv6AddressRange& other) const
    {

        return prefix.getPrefix(prefixLength) == other.prefix.getPrefix(prefixLength)&& (prefixLength <= other.prefixLength);
    }

    bool containsRange(const Ipv6Address& otherAddress, const int otherMask) const
    {
        return prefix.getPrefix(prefixLength) == otherAddress.getPrefix(prefixLength) && (prefixLength <= otherMask);
    }

    bool containedByRange(const Ipv6Address& otherAddress, const int otherMask) const
    {
        return prefix.getPrefix(otherMask) == otherAddress.getPrefix(otherMask) && (otherMask <= prefixLength);
    }

    bool operator!=(Ipv6AddressRange other) const
    {
        return !operator==(other);
    }

    std::string str() const;
};

inline std::string Ipv6AddressRange::str() const
{
    std::string str(prefix.str());
    str += "/";
    str += prefixLength;
    return str;
}

const Ipv6AddressRange NULL_IPV6ADDRESSRANGE(Ipv6Address(0, 0, 0, 0), 0);

inline bool isSameNetwork(Ipv6Address address1, int prefixLen1, Ipv6Address address2, int prefixLen2)
{
    return (prefixLen1 == prefixLen2) && (address1.getPrefix(prefixLen1) == address2.getPrefix(prefixLen2) );
}

//-------------------------------------------------------------------------------------
// Address range for IPv4
struct Ipv4AddressRange
{
    Ipv4Address address;
    Ipv4Address mask;
    Ipv4AddressRange() : address(), mask() {}
    Ipv4AddressRange(Ipv4Address addressPar, Ipv4Address maskPar) : address(addressPar), mask(maskPar) {}

    bool operator<(const Ipv4AddressRange& other) const
    {
        return (mask > other.mask) || ((mask == other.mask) && (address < other.address));
    }

    bool operator==(const Ipv4AddressRange& other) const
    {
        return (address == other.address) && (mask == other.mask);
    }

    bool contains(const Ipv4Address& other) const
    {
        return Ipv4Address::maskedAddrAreEqual(address, other, mask);
    }

    bool contains(const Ipv4AddressRange& other) const
    {
        return Ipv4Address::maskedAddrAreEqual(address, other.address, mask) && (mask <= other.mask);
    }

    bool containsRange(const Ipv4Address& otherAddress, const Ipv4Address& otherMask) const
    {
        return Ipv4Address::maskedAddrAreEqual(address, otherAddress, mask) && (mask <= otherMask);
    }

    bool containedByRange(const Ipv4Address& otherAddress, const Ipv4Address& otherMask) const
    {
        return Ipv4Address::maskedAddrAreEqual(otherAddress, address, otherMask) && (otherMask <= mask);
    }

    bool operator!=(Ipv4AddressRange other) const
    {
        return !operator==(other);
    }

    std::string str() const;
};

inline std::string Ipv4AddressRange::str() const
{
    std::string str(address.str(false));
    str += "/";
    str += mask.str(false);
    return str;
}

const Ipv4AddressRange NULL_IPV4ADDRESSRANGE(Ipv4Address(0, 0, 0, 0), Ipv4Address(0, 0, 0, 0));

inline Ipv4Address operator&(Ipv4Address address, Ipv4Address mask)
{
    Ipv4Address maskedAddress;
    maskedAddress.set(address.getInt() & mask.getInt());
    return maskedAddress;
}

inline Ipv4Address operator|(Ipv4Address address, Ipv4Address match)
{
    Ipv4Address matchAddress;
    matchAddress.set(address.getInt() | match.getInt());
    return matchAddress;
}

inline bool isSameNetwork(Ipv4Address address1, Ipv4Address mask1, Ipv4Address address2, Ipv4Address mask2)
{
    return (mask1 == mask2) && ((address1 & mask1) == (address2 & mask2));
}

//individual LSAs are identified by a combination
//of their LS type, Link State ID, and Advertising Router fields
struct LSAKeyType
{
    uint16_t LSType;
    Ipv4Address linkStateID;
    Ipv4Address advertisingRouter;
};

//Things needed for SPF Tree Vertices
struct NextHop
{
    int ifIndex;
    //Ipv6Address hopAddress;
    L3Address hopAddress;
    Ipv4Address advertisingRouter;      // Router ID
};

struct VertexID {
    int interfaceID=-1; //Needed only for Network Vertex
    Ipv4Address routerID;
};

inline bool operator==(const VertexID& leftID, const VertexID& rightID)
{
    return ((leftID.interfaceID == rightID.interfaceID)
            && (leftID.routerID == rightID.routerID));
}

enum InstallSource
{
    ORIGINATED = 0,
    FLOODED = 1
};

struct VertexLSA
{
    Ospfv3RouterLsa* routerLSA;
    Ospfv3NetworkLsa* networkLSA;
};

inline bool operator==(const NextHop& leftHop, const NextHop& rightHop)
{
    return (leftHop.ifIndex == rightHop.ifIndex) &&
           (leftHop.hopAddress == rightHop.hopAddress) &&
           (leftHop.advertisingRouter == rightHop.advertisingRouter);
}

inline bool operator!=(const NextHop& leftHop, const NextHop& rightHop)
{
    return !(leftHop == rightHop);
}

struct Ospfv3DdPacketId
{
    Ospfv3DdOptions ddOptions;
    Ospfv3Options options;
    unsigned long sequenceNumber;
};

inline bool operator != (Ospfv3DdOptions leftID, Ospfv3DdOptions rightID)
{
    return ((leftID.iBit != rightID.iBit) ||
            (leftID.mBit != rightID.mBit) ||
            (leftID.msBit != rightID.msBit));
}

inline bool operator == (Ospfv3Options leftID, Ospfv3Options rightID)
{
    return ((leftID.dcBit == rightID.dcBit) ||
            (leftID.eBit == rightID.eBit) ||
            (leftID.nBit == rightID.nBit) ||
            (leftID.rBit == rightID.rBit) ||
            (leftID.v6Bit == rightID.v6Bit) ||
            (leftID.xBit == rightID.xBit));
}

inline bool operator != (Ospfv3Options leftID, Ospfv3Options rightID)
{
    return !(leftID==rightID);
}

inline bool operator !=(Ospfv3DdPacketId leftID, Ospfv3DdPacketId rightID)
{
    return ((leftID.ddOptions.iBit != rightID.ddOptions.iBit) ||
       (leftID.ddOptions.mBit != rightID.ddOptions.mBit) ||
       (leftID.ddOptions.msBit != rightID.ddOptions.msBit) ||
       (leftID.options.dcBit != rightID.options.dcBit) ||
       (leftID.options.eBit != rightID.options.eBit) ||
       (leftID.options.nBit != rightID.options.nBit) ||
       (leftID.options.rBit != rightID.options.rBit) ||
       (leftID.options.v6Bit != rightID.options.v6Bit) ||
       (leftID.options.xBit != rightID.options.xBit) ||
       (leftID.sequenceNumber != rightID.sequenceNumber));
}

inline bool operator<(Ospfv3LsaHeader& leftLSA, Ospfv3LsaHeader& rightLSA)
{
    long leftSequenceNumber = leftLSA.getLsaSequenceNumber();
    long rightSequenceNumber = rightLSA.getLsaSequenceNumber();

    if (leftSequenceNumber < rightSequenceNumber) {
        return true;
    }
    if (leftSequenceNumber == rightSequenceNumber) {
        unsigned short leftAge = leftLSA.getLsaAge();
        unsigned short rightAge = rightLSA.getLsaAge();

        if ((leftAge != MAX_AGE) && (rightAge == MAX_AGE)) {
            return true;
        }
        if ((leftAge == MAX_AGE) && (rightAge != MAX_AGE)) {
            return false;
        }
        if ((abs(leftAge - rightAge) > MAX_AGE_DIFF) && (leftAge > rightAge)) {
            return true;
        }
    }
    return false;
}

inline bool operator<(const Ospfv3LsaHeader& leftLSA, const Ospfv3LsaHeader& rightLSA)
{
    long leftSequenceNumber = leftLSA.getLsaSequenceNumber();
    long rightSequenceNumber = rightLSA.getLsaSequenceNumber();

    if (leftSequenceNumber < rightSequenceNumber) {
        return true;
    }
    if (leftSequenceNumber == rightSequenceNumber) {
        unsigned short leftAge = leftLSA.getLsaAge();
        unsigned short rightAge = rightLSA.getLsaAge();

        if ((leftAge != MAX_AGE) && (rightAge == MAX_AGE)) {
            return true;
        }
        if ((leftAge == MAX_AGE) && (rightAge != MAX_AGE)) {
            return false;
        }
        if ((abs(leftAge - rightAge) > MAX_AGE_DIFF) && (leftAge > rightAge)) {
            return true;
        }
    }
    return false;
}

inline bool operator==(Ospfv3LsaHeader& leftLSA, Ospfv3LsaHeader& rightLSA)
{
    long leftSequenceNumber = leftLSA.getLsaSequenceNumber();
    long rightSequenceNumber = rightLSA.getLsaSequenceNumber();
    unsigned short leftAge = leftLSA.getLsaAge();
    unsigned short rightAge = rightLSA.getLsaAge();

    if ((leftSequenceNumber == rightSequenceNumber) &&
        (((leftAge == MAX_AGE) && (rightAge == MAX_AGE)) ||
         (((leftAge != MAX_AGE) && (rightAge != MAX_AGE)) &&
          (abs(leftAge - rightAge) <= MAX_AGE_DIFF))))
    {
        return true;
    }
    else {
        return false;
    }
}

inline bool operator==(const Ospfv3LsaHeader& leftLSA, const Ospfv3LsaHeader& rightLSA)
{
    long leftSequenceNumber = leftLSA.getLsaSequenceNumber();
    long rightSequenceNumber = rightLSA.getLsaSequenceNumber();
    unsigned short leftAge = leftLSA.getLsaAge();
    unsigned short rightAge = rightLSA.getLsaAge();

    if ((leftSequenceNumber == rightSequenceNumber) &&
        (((leftAge == MAX_AGE) && (rightAge == MAX_AGE)) ||
         (((leftAge != MAX_AGE) && (rightAge != MAX_AGE)) &&
          (abs(leftAge - rightAge) <= MAX_AGE_DIFF))))
    {
        return true;
    }
    else {
        return false;
    }
}
//Packets

} //namespace ospfv3
}//namespace inet

#endif // __INET_OSPFV3COMMON_H_

