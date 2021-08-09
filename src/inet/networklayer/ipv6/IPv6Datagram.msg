//
// Copyright (C) 2005 Wei Yang, Ng
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2001-2004 CTIE, Monash University
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

import inet.networklayer.common.IPProtocolId;
import inet.networklayer.contract.ipv6.IPv6Address;
import inet.networklayer.contract.ipv6.IPv6ExtensionHeaders;

cplusplus {{
#include <iostream>
#include "inet/networklayer/ipv6/IPv6ExtensionHeaders.h"
}}

namespace inet;

cplusplus {{
const int IPv6_HEADER_BYTES = 40;
}}


enum IPProtocolId;

//
// IPv6 datagram. RFC 2460 Section 3.
//
// Header fields not explicitly modelled:
//    - version (=6): represented by the class name itself
//    - payload length: will be calculated from encapsulated message length
//      and extension headers' length
//
packet IPv6Datagram
{
    @customize(true);
    IPv6Address srcAddress;
    IPv6Address destAddress;
    unsigned char trafficClass;
    abstract int diffServCodePoint;  // maps to bits 0-5 of trafficClass
    abstract int explicitCongestionNotification;  // maps to bits 6-7 of trafficClass
    unsigned int flowLabel;
    short hopLimit = 0;
    int transportProtocol @enum(IPProtocolId);

    IPv6ExtensionHeader * extensionHeader[] @owned; // array of extension headers, subclassed from ~IPv6ExtensionHeader
}

