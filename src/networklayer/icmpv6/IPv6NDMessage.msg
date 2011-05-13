//
// Copyright (C) 2005 Wei Yang, Ng
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

cplusplus {{
#include "ICMPv6Message_m.h"
#include "IPv6Address.h"
#include "MACAddress.h"
}}


packet ICMPv6Message;

class noncobject IPv6Address;

class noncobject MACAddress;

//
// IPv6ND Prefix Information
// RFC 2461 / RFC 4861 Section 4.6.2
// RFC 3775 Section 7.2 (routerAddressFlag)
//
class IPv6NDPrefixInformation
{
    unsigned short prefixLength;
    bool onlinkFlag;        //L-bit
    bool autoAddressConfFlag;    //A-bit
    bool routerAddressFlag;        //R-bit: used in case of MIPv6 when the H-bit is set
    unsigned int validLifetime;    // seconds
    unsigned int preferredLifetime;    // seconds
    IPv6Address prefix;
}

// MIPv6 New Advertisement Interval Option
// RFC 3775 Section 7.3
//
class MIPv6NDAdvertisementInterval
{
    unsigned int advertisementInterval;    // milliseconds
}

//MIPv6 Home Agent Information Option
//RFC 3775 Section 7.4
class MIPv6HAInformation
{
    unsigned int homeAgentPreference;
    unsigned int homeAgentLifetime;    // seconds
}

//
// Neighbour Discovery for IPv6.
// RFC 2461
//
// ICMP fields inherited from ~ICMPv6Message:
//    - Type
//
// ICMP fields not implemented:
//    - Checksum
//    - Reserved
//
packet IPv6NDMessage extends ICMPv6Message
{
    //customize=true;
    int code = 0;
}

//
// Router Solicitation Message Format
// RFC 2461 Section 4.1
//
packet IPv6RouterSolicitation extends IPv6NDMessage
{
    //Possible Options
    MACAddress sourceLinkLayerAddress;
}

//
// Router Advertisement Message Format
// RFC 2461 Section 4.2
// RFC 3775 Section 7.1, RFC 5175 Section 3. (homeAgentFlag)
//
packet IPv6RouterAdvertisement extends IPv6NDMessage
{
    //Additional ICMP fields
    unsigned short curHopLimit;
    bool managedAddrConfFlag;    //M-bit
    bool otherStatefulConfFlag;    //O-bit
    bool homeAgentFlag = false;        //H-bit
    unsigned short routerLifetime;  // 0 indicates router is not a default router
    unsigned int reachableTime;
    unsigned int retransTimer;

    //Possible Options
    MACAddress sourceLinkLayerAddress;        //source link layer option
    unsigned int MTU;                //MTU option
    IPv6NDPrefixInformation prefixInformation[];    //Prefix Information Option
    MIPv6NDAdvertisementInterval advInterval;    //New Advertisement Interval Option (MIPv6)
    MIPv6HAInformation haInformation;    //New Home Agent Information Option (MIPv6)
}

//
// Neighbour Solicitation Message Format
// RFC 2461 Section 4.3
//
packet IPv6NeighbourSolicitation extends IPv6NDMessage
{
    //Additional ICMP fields
    IPv6Address targetAddress;// MUST NOT be a multicast address.

    //Possible Options
    MACAddress sourceLinkLayerAddress;
}

//
// Neighbour Advertisement Message Format
// RFC 2461 Section 4.4
//
packet IPv6NeighbourAdvertisement extends IPv6NDMessage
{
    //Additional ICMP fields
    bool routerFlag;    //R-flag
    bool solicitedFlag;    //S-flag
    bool overrideFlag;    //O-flag
    IPv6Address targetAddress;// MUST NOT be a multicast address.

    //Possible Options
    MACAddress targetLinkLayerAddress;    //Target Link Layer Address option
}

//
// Redirect Message Format
// RFC 2461 Section 4.5
//
packet IPv6Redirect extends IPv6NDMessage
{
    //Additional ICMP fields
    IPv6Address targetAddress;
    IPv6Address destinationAddress;

    //Possible Options
    MACAddress targetLinkLayerAddress;
        //Redirected Header Encapsulated Msg
}
