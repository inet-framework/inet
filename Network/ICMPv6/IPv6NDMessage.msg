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
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

cplusplus {{
#include "ICMPv6Message_m.h"
#include "IPv6Address.h"
#include "MACAddress.h"
}}

message ICMPv6Message;
class noncobject IPv6Address;
class noncobject MACAddress;

//
// IPv6ND Prefix Information
// RFC 2461 Section 4.6.2
//
class IPv6NDPrefixInformation
{
    properties:
        omitGetVerb = true;
    fields:
        unsigned int prefixLength;
        bool onlinkFlag;
        bool autoAddressConfFlag;
        unsigned int validLifetime;
        unsigned int preferredLifetime;
        IPv6Address prefix;
}

//
// Neighbour Discovery for IPv6.
// RFC 2461
//
// ICMP fields inherited from ICMPv6Message:
//    - Type
//
// ICMP fields not implemented:
//    - Checksum
//    - Reserved
//
message IPv6NDMessage extends ICMPv6Message
{
    properties:
        //customize=true;
        omitGetVerb = true;
    fields:
        int code = 0;
};

//
// Router Solicitation Message Format
// RFC 2461 Section 4.1
//
message IPv6RouterSolicitation extends IPv6NDMessage
{
    properties:
        omitGetVerb = true;
    fields:
        //Possible Options
        MACAddress sourceLinkLayerAddress;
};

//
// Router Advertisement Message Format
// RFC 2461 Section 4.2
//
message IPv6RouterAdvertisement extends IPv6NDMessage
{
    properties:
        omitGetVerb = true;
    fields:
        //Additional ICMP fields
        unsigned int curHopLimit;
        bool managedAddrConfFlag;
        bool otherStatefulConfFlag;
        unsigned short routerLifetime;  // 0 indicates router is not a default router
        unsigned int reachableTime;
        unsigned int retransTimer;

        //Possible Options
        MACAddress sourceLinkLayerAddress;
        unsigned int MTU;
        IPv6NDPrefixInformation prefixInformation[];
};

//
// Neighbour Solicitation Message Format
// RFC 2461 Section 4.3
//
message IPv6NeighbourSolicitation extends IPv6NDMessage
{
    properties:
        omitGetVerb = true;
    fields:
        //Additional ICMP fields
        IPv6Address targetAddress;// MUST NOT be a multicast address.

        //Possible Options
        MACAddress sourceLinkLayerAddress;
};

//
// Neighbour Advertisement Message Format
// RFC 2461 Section 4.4
//
message IPv6NeighbourAdvertisement extends IPv6NDMessage
{
    properties:
        omitGetVerb = true;
    fields:
        //Additional ICMP fields
        bool routerFlag;
        bool solicitedFlag;
        bool overrideFlag;
        IPv6Address targetAddress;// MUST NOT be a multicast address.

        //Possible Options
        MACAddress targetLinkLayerAddress;
};

//
// Redirect Message Format
// RFC 2461 Section 4.5
//
message IPv6Redirect extends IPv6NDMessage
{
    properties:
        omitGetVerb = true;
    fields:
        //Additional ICMP fields
        IPv6Address targetAddress;
        IPv6Address destinationAddress;

        //Possible Options
        MACAddress targetLinkLayerAddress;
        //Redirected Header Encapsulated Msg
};
