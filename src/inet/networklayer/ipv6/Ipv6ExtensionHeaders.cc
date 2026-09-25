//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders.h"

namespace inet {

std::ostream& operator<<(std::ostream& os, Ipv6ExtensionHeader *eh)
{
    return os << "(" << eh->getClassName() << ") " << eh->str();
}

std::ostream& operator<<(std::ostream& out, const Ipv6ExtensionHeader& h)
{
    out << "{type:" << h.getExtensionType() << ",length:" << h.getChunkLength() << "}";
    return out;
}

/*
 *  Defines the order of extension headers according to RFC 2460 4.1.
 *  Note that Destination Options header may come both after the Hop-by-Hop
 *  extension and before the payload (if Routing presents).
 */
int Ipv6ExtensionHeader::getOrder() const
{
    switch (extensionType) {
        case IP_PROT_IPv6EXT_HOP:
            return 0;

        case IP_PROT_IPv6EXT_DEST:
            return 1;

        case IP_PROT_IPv6EXT_ROUTING:
            return 2;

        case IP_PROT_IPv6EXT_FRAGMENT:
            return 3;

        case IP_PROT_IPv6EXT_AUTH:
            return 4;

        case IP_PROT_IPv6EXT_ESP:
            return 5;

        // second IP_PROT_IPv6EXT_DEST has order 6

        case IP_PROT_IPv6EXT_MOB:
            return 7;

        default:
            return -1;
    }
}

Ipv6HopByHopOptionsHeader *createIpv6RouterAlertHeader(uint16_t value)
{
    auto header = new Ipv6HopByHopOptionsHeader();
    auto routerAlert = new Ipv6RouterAlertOption();
    routerAlert->setValue(value);
    header->getTlvOptionsForUpdate().appendTlvOption(routerAlert);
    // a PadN option without data fills the header to 8 octets: 2 + 4 + 2
    auto padding = new TlvOptionBase();
    padding->setType(IPv6TLVOPTION_NOPN);
    padding->setLength(0);
    header->getTlvOptionsForUpdate().appendTlvOption(padding);
    header->setChunkLength(B(8));
    return header;
}

} // namespace inet
