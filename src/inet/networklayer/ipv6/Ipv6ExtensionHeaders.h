//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV6EXTENSIONHEADERS_H
#define __INET_IPV6EXTENSIONHEADERS_H

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders_m.h"

namespace inet {

/**
 * Returns true if the given protocol ID corresponds to an IPv6 extension header
 * that the IPv6 core walks/processes.
 *
 * AH (51) and ESP (50) are deliberately excluded: the IPv6 core does not implement
 * them, so it treats them as terminal (upper-layer) headers that end the extension
 * header chain. The IPsec module (a netfilter hook) strips AH/ESP before the chain is
 * walked; if no IPsec module is present, AH/ESP are dispatched to the registered
 * AH/ESP protocol (dissector) like any other upper-layer protocol.
 */
inline bool isIpv6ExtensionHeader(IpProtocolId id)
{
    switch (id) {
        case IP_PROT_IPv6EXT_HOP:
        case IP_PROT_IPv6EXT_DEST:
        case IP_PROT_IPv6EXT_ROUTING:
        case IP_PROT_IPv6EXT_FRAGMENT:
            return true;
        default:
            return false;
    }
}

/**
 * Peeks the correct concrete Ipv6ExtensionHeader subclass at the given offset
 * based on the protocol ID. This is needed because the abstract base class
 * Ipv6ExtensionHeader has no registered serializer, so peekAt<Ipv6ExtensionHeader>()
 * would fail on serialized (network-received) packets.
 */
inline Ptr<const Ipv6ExtensionHeader> peekIpv6ExtensionHeaderAt(const Packet *packet, b offset, IpProtocolId protocolId)
{
    switch (protocolId) {
        case IP_PROT_IPv6EXT_HOP:
            return packet->peekDataAt<Ipv6HopByHopOptionsHeader>(offset);
        case IP_PROT_IPv6EXT_DEST:
            return packet->peekDataAt<Ipv6DestinationOptionsHeader>(offset);
        case IP_PROT_IPv6EXT_ROUTING:
            return packet->peekDataAt<Ipv6RoutingHeader>(offset);
        case IP_PROT_IPv6EXT_FRAGMENT:
            return packet->peekDataAt<Ipv6FragmentHeader>(offset);
        case IP_PROT_IPv6EXT_AUTH:
            return packet->peekDataAt<Ipv6AuthenticationHeader>(offset);
        case IP_PROT_IPv6EXT_ESP:
            return packet->peekDataAt<Ipv6EncapsulatingSecurityPayloadHeader>(offset);
        default:
            throw cRuntimeError("peekIpv6ExtensionHeaderAt: unknown extension header protocol %d", (int)protocolId);
    }
}

/**
 * Walks the extension headers that follow an IPv6 header whose Next Header field is
 * nextHeader, starting at the given offset of the packet, and returns the protocol of
 * the upper-layer header (RFC 8200 Section 4). The offset is advanced past the
 * extension headers. Returns IP_PROT_NONE when the upper-layer header is not in the
 * packet: the packet is a fragment other than the first (RFC 8200 Section 4.5), or it
 * ends inside the extension header chain, as the quoted packet of an ICMPv6 error may.
 */
inline IpProtocolId skipIpv6ExtensionHeaders(const Packet *packet, b& offset, IpProtocolId nextHeader)
{
    while (isIpv6ExtensionHeader(nextHeader)) {
        if (offset >= packet->getDataLength())
            return IP_PROT_NONE;
        auto extensionHeader = peekIpv6ExtensionHeaderAt(packet, offset, nextHeader);
        if (nextHeader == IP_PROT_IPv6EXT_FRAGMENT && staticPtrCast<const Ipv6FragmentHeader>(extensionHeader)->getFragmentOffset() != 0)
            return IP_PROT_NONE;
        offset += extensionHeader->getChunkLength();
        nextHeader = extensionHeader->getNextHeaderProtocol();
    }
    return nextHeader;
}

} // namespace inet

#endif
