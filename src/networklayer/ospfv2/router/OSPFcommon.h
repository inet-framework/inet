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

#ifndef __INET_OSPFCOMMON_H
#define __INET_OSPFCOMMON_H

#include <ctype.h>
#include <stdio.h>
#include <functional>

#include "IPv4Address.h"


// global constants
#define LS_REFRESH_TIME                     1800
#define MIN_LS_INTERVAL                     5
#define MIN_LS_ARRIVAL                      1
#define MAX_AGE                             3600
#define CHECK_AGE                           300
#define MAX_AGE_DIFF                        900
#define LS_INFINITY                         16777215
#define DEFAULT_DESTINATION_ADDRESS         0
#define DEFAULT_DESTINATION_MASK            0
#define INITIAL_SEQUENCE_NUMBER             -2147483647
#define MAX_SEQUENCE_NUMBER                 2147483647

#define VIRTUAL_LINK_TTL                    32
#define IPV4_HEADER_LENGTH                  60
#define IPV4_DATAGRAM_LENGTH                65536
#define OSPF_HEADER_LENGTH                  24
#define OSPF_LSA_HEADER_LENGTH              20
#define OSPF_DD_HEADER_LENGTH               8
#define OSPF_REQUEST_LENGTH                 12
#define OSPF_ROUTERLSA_HEADER_LENGTH        4
#define OSPF_LINK_HEADER_LENGTH             12
#define OSPF_TOS_LENGTH                     4
#define OSPF_NETWORKLSA_MASK_LENGTH         4
#define OSPF_NETWORKLSA_ADDRESS_LENGTH      4
#define OSPF_SUMMARYLSA_HEADER_LENGTH       8
#define OSPF_ASEXTERNALLSA_HEADER_LENGTH    16
#define OSPF_ASEXTERNALLSA_TOS_INFO_LENGTH  12
#define OSPF_EXTERNAL_ROUTES_LEARNED_BY_BGP 179
#define OSPF_BGP_DEFAULT_COST               1

namespace OSPF {

typedef unsigned long Metric;

enum AuthenticationType {
    NULL_TYPE = 0,
    SIMPLE_PASSWORD_TYPE = 1,
    CRYTOGRAPHIC_TYPE = 2
};

struct AuthenticationKeyType {
    char    bytes[8];
};

////FIXME remove this type, use IPv4Address instead
//struct IPv4Address {
//    unsigned char   bytes[4];
//
//
//    unsigned int asInt() { return (bytes[0]<<24) | bytes[1]<<16 | bytes[2]<<8 | bytes[3]; }
//};

class IPv4Address_Less : public std::binary_function <IPv4Address, IPv4Address, bool>
{
public:
    bool operator() (IPv4Address leftAddress, IPv4Address rightAddress) const;
};

struct IPv4AddressRange {
    IPv4Address address;
    IPv4Address mask;
};

class IPv4AddressRange_Less : public std::binary_function <IPv4AddressRange, IPv4AddressRange, bool>
{
public:
    bool operator() (IPv4AddressRange leftAddressRange, IPv4AddressRange rightAddressRange) const;
};

struct HostRouteParameters {
    unsigned char ifIndex;
    IPv4Address   address;
    Metric        linkCost;
};

typedef unsigned long RouterID;
typedef unsigned long AreaID;
typedef unsigned long LinkStateID;

struct LSAKeyType {
    LinkStateID linkStateID;
    RouterID    advertisingRouter;
};

class LSAKeyType_Less : public std::binary_function <LSAKeyType, LSAKeyType, bool>
{
public:
    bool operator() (LSAKeyType leftKey, LSAKeyType rightKey) const;
};

struct DesignatedRouterID {
    RouterID    routerID;
    IPv4Address ipInterfaceAddress;
};

const RouterID              NULL_ROUTERID = 0;
const AreaID                BACKBONE_AREAID = 0;
const LinkStateID           NULL_LINKSTATEID = 0;
const IPv4Address           NULL_IPV4ADDRESS(0, 0, 0, 0);
const IPv4Address           ALL_SPF_ROUTERS(224, 0, 0, 5);
const IPv4Address           ALL_D_ROUTERS(224, 0, 0, 6);
const IPv4AddressRange      NULL_IPV4ADDRESSRANGE = { IPv4Address(0, 0, 0, 0), IPv4Address(0, 0, 0, 0)};
const DesignatedRouterID    NULL_DESIGNATEDROUTERID = { 0, IPv4Address(0, 0, 0, 0)};

} // namespace OSPF

inline IPv4Address operator&(IPv4Address address, IPv4Address mask)
{
    IPv4Address maskedAddress;
    maskedAddress.set(address.getInt() & mask.getInt());
    return maskedAddress;
}

inline IPv4Address operator|(IPv4Address address, IPv4Address match)
{
    IPv4Address matchAddress;
    matchAddress.set(address.getInt() | match.getInt());
    return matchAddress;
}

inline bool operator==(OSPF::IPv4AddressRange leftAddressRange, OSPF::IPv4AddressRange rightAddressRange)
{
    return (leftAddressRange.address == rightAddressRange.address &&
            leftAddressRange.mask == rightAddressRange.mask);
}

inline bool operator!=(OSPF::IPv4AddressRange leftAddressRange, OSPF::IPv4AddressRange rightAddressRange)
{
    return (!(leftAddressRange == rightAddressRange));
}

inline bool operator==(OSPF::DesignatedRouterID leftID, OSPF::DesignatedRouterID rightID)
{
    return (leftID.routerID == rightID.routerID &&
            leftID.ipInterfaceAddress == rightID.ipInterfaceAddress);
}

inline bool operator!=(OSPF::DesignatedRouterID leftID, OSPF::DesignatedRouterID rightID)
{
    return (!(leftID == rightID));
}

inline bool OSPF::IPv4Address_Less::operator() (IPv4Address leftAddress, IPv4Address rightAddress) const
{
    return (leftAddress < rightAddress);
}

inline bool OSPF::IPv4AddressRange_Less::operator() (OSPF::IPv4AddressRange leftAddressRange, OSPF::IPv4AddressRange rightAddressRange) const
{
    return ((leftAddressRange.address < rightAddressRange.address) ||
            ((leftAddressRange.address == rightAddressRange.address) &&
             (leftAddressRange.mask < rightAddressRange.mask)));
}

inline bool OSPF::LSAKeyType_Less::operator() (OSPF::LSAKeyType leftKey, OSPF::LSAKeyType rightKey) const
{
    return ((leftKey.linkStateID < rightKey.linkStateID) ||
            ((leftKey.linkStateID == rightKey.linkStateID) &&
             (leftKey.advertisingRouter < rightKey.advertisingRouter)));
}

inline IPv4Address ipv4AddressFromAddressString(const char* charForm)
{
    return IPv4Address(charForm);
}

inline IPv4Address ipv4AddressFromULong(unsigned long longForm)
{
    return IPv4Address(longForm);
}

inline unsigned long ulongFromIPv4Address(IPv4Address byteForm)
{
    return byteForm.getInt();
}

inline unsigned long ulongFromAddressString(const char* charForm)
{
    return ulongFromIPv4Address(ipv4AddressFromAddressString(charForm));
}

inline char* addressStringFromIPv4Address(char* buffer, int bufferLength, IPv4Address byteForm)
{
    if (bufferLength < 16)
        buffer = '\0';
    else
        sprintf(buffer, "%s", byteForm.str().c_str());

    return buffer;
}

inline char* addressStringFromULong(char* buffer, int bufferLength, unsigned long longForm)
{
    if (bufferLength < 16) {
        buffer = '\0';
    }
    else {
        sprintf(buffer, "%d.%d.%d.%d", (int)((longForm & 0xFF000000) >> 24),
                                        (int)((longForm & 0x00FF0000) >> 16),
                                        (int)((longForm & 0x0000FF00) >> 8),
                                        (int)(longForm & 0x000000FF));
    }
    return buffer;
}

inline char hexCharToByte(char hex)
{
    switch (hex) {
        case '0':   return 0;
        case '1':   return 1;
        case '2':   return 2;
        case '3':   return 3;
        case '4':   return 4;
        case '5':   return 5;
        case '6':   return 6;
        case '7':   return 7;
        case '8':   return 8;
        case '9':   return 9;
        case 'A':   return 10;
        case 'B':   return 11;
        case 'C':   return 12;
        case 'D':   return 13;
        case 'E':   return 14;
        case 'F':   return 15;
        case 'a':   return 10;
        case 'b':   return 11;
        case 'c':   return 12;
        case 'd':   return 13;
        case 'e':   return 14;
        case 'f':   return 15;
        default:    return 0;
    };
}

inline char hexPairToByte(char upperHex, char lowerHex)
{
    return ((hexCharToByte(upperHex) << 4) & (hexCharToByte(lowerHex)));
}

#endif // __COMMON_HPP__

