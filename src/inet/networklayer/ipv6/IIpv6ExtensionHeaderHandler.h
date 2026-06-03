//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IIPV6EXTENSIONHEADERHANDLER_H
#define __INET_IIPV6EXTENSIONHEADERHANDLER_H

#include "inet/common/INETDefs.h"

namespace inet {

class Packet;
class Ipv6ExtensionHeader;
class TlvOptionBase;

// For whole extension headers: Routing Header (by routing type),
// and generic extension headers registered by NH value (AH, ESP, etc.)
class INET_API IIpv6ExtensionHeaderHandler {
public:
    virtual ~IIpv6ExtensionHeaderHandler() = default;
    // Process the given extension header. May modify the packet.
    // Returns true = continue processing next headers / local delivery.
    // Returns false = packet was consumed (dropped or error sent).
    virtual bool processExtensionHeader(Packet *packet, const Ipv6ExtensionHeader *eh) = 0;
};

// For TLV options within Hop-by-Hop or Destination Options headers
class INET_API IIpv6TlvOptionHandler {
public:
    virtual ~IIpv6TlvOptionHandler() = default;
    // Process the given TLV option. May modify the packet.
    // Returns true = continue processing next options / headers.
    // Returns false = packet was consumed (dropped or error sent).
    virtual bool processTlvOption(Packet *packet, const Ipv6ExtensionHeader *eh, const TlvOptionBase *option) = 0;
};

}  // namespace inet

#endif
