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

#ifndef __INET_OSPFV2COMMON_H
#define __INET_OSPFV2COMMON_H

#include <ctype.h>
#include <functional>
#include <stdio.h>

#include "inet/common/INETDefs.h"
#include "inet/common/Units_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"

namespace inet {

namespace ospfv2 {

// global constants
#define LS_REFRESH_TIME                        1800
#define MIN_LS_INTERVAL                        5
#define MIN_LS_ARRIVAL                         1
#define MAX_AGE                                3600
#define CHECK_AGE                              300
#define MAX_AGE_DIFF                           900
#define LS_INFINITY                            16777215
#define DEFAULT_DESTINATION_ADDRESS            0
#define DEFAULT_DESTINATION_MASK               0
#define INITIAL_SEQUENCE_NUMBER                -2147483647
#define MAX_SEQUENCE_NUMBER                    2147483647

#define VIRTUAL_LINK_TTL                       32

const B IPV4_DATAGRAM_LENGTH                   = B(65536);
const B OSPFv2_HEADER_LENGTH                   = B(24);
const B OSPFv2_LSA_HEADER_LENGTH               = B(20);
const B OSPFv2_HELLO_HEADER_LENGTH             = B(20);
const B OSPFv2_DD_HEADER_LENGTH                = B(8);
const B OSPFv2_REQUEST_LENGTH                  = B(12);
const B OSPFv2_ROUTERLSA_HEADER_LENGTH         = B(4);
const B OSPFv2_LINK_HEADER_LENGTH              = B(12);
const B OSPFv2_TOS_LENGTH                      = B(4);
const B OSPFv2_NETWORKLSA_MASK_LENGTH          = B(4);
const B OSPFv2_NETWORKLSA_ADDRESS_LENGTH       = B(4);
const B OSPFv2_SUMMARYLSA_HEADER_LENGTH        = B(8);
const B OSPFv2_ASEXTERNALLSA_HEADER_LENGTH     = B(4);
const B OSPFv2_ASEXTERNALLSA_TOS_INFO_LENGTH   = B(12);

#define OSPFv2_EXTERNAL_ROUTES_LEARNED_BY_BGP  179
#define OSPFv2_BGP_DEFAULT_COST                1

typedef unsigned long Metric;

enum AuthenticationType {
    NULL_TYPE = 0,
    SIMPLE_PASSWORD_TYPE = 1,
    CRYTOGRAPHIC_TYPE = 2
};

struct AuthenticationKeyType
{
    char bytes[8];
};

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

struct HostRouteParameters
{
    unsigned char ifIndex;
    Ipv4Address address;
    Metric linkCost;
};

typedef Ipv4Address RouterId;
typedef Ipv4Address AreaId;
typedef Ipv4Address LinkStateId;

struct LsaKeyType
{
    LinkStateId linkStateID;
    RouterId advertisingRouter;
};

class LsaKeyType_Less : public std::binary_function<LsaKeyType, LsaKeyType, bool>
{
  public:
    bool operator()(LsaKeyType leftKey, LsaKeyType rightKey) const;
};

struct DesignatedRouterId
{
    RouterId routerID;
    Ipv4Address ipInterfaceAddress;
};

const RouterId NULL_ROUTERID(0, 0, 0, 0);
const AreaId BACKBONE_AREAID(0, 0, 0, 0);
const LinkStateId NULL_LINKSTATEID(0, 0, 0, 0);
const Ipv4Address NULL_IPV4ADDRESS(0, 0, 0, 0);
const Ipv4AddressRange NULL_IPV4ADDRESSRANGE(Ipv4Address(0, 0, 0, 0), Ipv4Address(0, 0, 0, 0));
const DesignatedRouterId NULL_DESIGNATEDROUTERID = {
    Ipv4Address(0, 0, 0, 0), Ipv4Address(0, 0, 0, 0)
};

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

inline bool operator==(DesignatedRouterId leftID, DesignatedRouterId rightID)
{
    return leftID.routerID == rightID.routerID &&
           leftID.ipInterfaceAddress == rightID.ipInterfaceAddress;
}

inline bool operator!=(DesignatedRouterId leftID, DesignatedRouterId rightID)
{
    return !(leftID == rightID);
}

inline bool LsaKeyType_Less::operator()(LsaKeyType leftKey, LsaKeyType rightKey) const
{
    return (leftKey.linkStateID < rightKey.linkStateID) ||
           ((leftKey.linkStateID == rightKey.linkStateID) &&
            (leftKey.advertisingRouter < rightKey.advertisingRouter));
}

inline Ipv4Address ipv4AddressFromAddressString(const char *charForm)
{
    return L3AddressResolver().resolve(charForm, L3AddressResolver::ADDR_IPv4).toIpv4();
}

inline Ipv4Address ipv4NetmaskFromAddressString(const char *charForm)
{
    return L3AddressResolver().resolve(charForm, L3AddressResolver::ADDR_IPv4 | L3AddressResolver::ADDR_MASK).toIpv4();
}

inline char hexCharToByte(char hex)
{
    switch (hex) {
        case '0':
            return 0;

        case '1':
            return 1;

        case '2':
            return 2;

        case '3':
            return 3;

        case '4':
            return 4;

        case '5':
            return 5;

        case '6':
            return 6;

        case '7':
            return 7;

        case '8':
            return 8;

        case '9':
            return 9;

        case 'A':
            return 10;

        case 'B':
            return 11;

        case 'C':
            return 12;

        case 'D':
            return 13;

        case 'E':
            return 14;

        case 'F':
            return 15;

        case 'a':
            return 10;

        case 'b':
            return 11;

        case 'c':
            return 12;

        case 'd':
            return 13;

        case 'e':
            return 14;

        case 'f':
            return 15;

        default:
            break;
    }
    ;
    return 0;
}

inline char hexPairToByte(char upperHex, char lowerHex)
{
    return (hexCharToByte(upperHex) << 4) & (hexCharToByte(lowerHex));
}

} // namespace ospfv2

} // namespace inet

#endif    // __COMMON_HPP__

