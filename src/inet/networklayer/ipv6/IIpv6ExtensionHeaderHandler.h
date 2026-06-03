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

/**
 * Handler interface for IPv6 extension headers that are dispatched as whole
 * headers (e.g. Routing Header by routing type).
 *
 * Modules that need to process specific extension header types register
 * themselves with the Ipv6 module via Ipv6::registerRoutingHeaderHandler().
 */
class INET_API IIpv6ExtensionHeaderHandler {
public:
    virtual ~IIpv6ExtensionHeaderHandler() = default;

    /**
     * Process the given extension header. May modify the packet (e.g. swap
     * addresses, decrement segments left).
     *
     * @return true if processing should continue (next headers / local delivery),
     *         false if the packet was consumed (dropped or error sent).
     */
    virtual bool processExtensionHeader(Packet *packet, const Ipv6ExtensionHeader *eh) = 0;
};

/**
 * Handler interface for individual TLV options within Hop-by-Hop Options or
 * Destination Options extension headers.
 *
 * Modules that need to process specific TLV option types register themselves
 * with the Ipv6 module via Ipv6::registerHopByHopOptionHandler() or
 * Ipv6::registerDestinationOptionHandler().
 */
class INET_API IIpv6TlvOptionHandler {
public:
    virtual ~IIpv6TlvOptionHandler() = default;

    /**
     * Process the given TLV option. May modify the packet (e.g. swap
     * source address with Home Address).
     *
     * @param eh  the enclosing extension header (Hop-by-Hop or Destination Options)
     * @return true if processing should continue (next options / headers),
     *         false if the packet was consumed (dropped or error sent).
     */
    virtual bool processTlvOption(Packet *packet, const Ipv6ExtensionHeader *eh, const TlvOptionBase *option) = 0;
};

}  // namespace inet

#endif
