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
 * Returns true if the given protocol ID corresponds to an IPv6 extension header.
 */
inline bool isIpv6ExtensionHeader(IpProtocolId id)
{
    switch (id) {
        case IP_PROT_IPv6EXT_HOP:
        case IP_PROT_IPv6EXT_DEST:
        case IP_PROT_IPv6EXT_ROUTING:
        case IP_PROT_IPv6EXT_FRAGMENT:
        case IP_PROT_IPv6EXT_AUTH:
        case IP_PROT_IPv6EXT_ESP:
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

} // namespace inet

#endif
