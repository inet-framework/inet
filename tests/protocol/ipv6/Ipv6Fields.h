//
// Helpers for the IPv6 level 2 tests: read an address field of an observed packet, resolve
// a node's address, and read the UDP header behind a fragment header.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_IPV6FIELDS_H
#define __INET_PROTOCOLTEST_IPV6FIELDS_H

#include <string>

#include "PacketField.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders_m.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {
namespace protocoltest {

// An IPv6 address field of an observed packet as text, e.g. ipv6Address(p, "ipv6.srcAddress").
inline std::string ipv6Address(const Packet *packet, const char *fieldPath)
{
    return evalPacketField(packet, fieldPath).pointerValue().get<Ipv6Address>()->str();
}

// The routable IPv6 address of a node of the network, as text.
inline std::string nodeIpv6Address(const char *node)
{
    return L3AddressResolver().resolve(node, L3AddressResolver::ADDR_IPv6).toIpv6().str();
}

// The payload length field of the IPv6 base header of an observed frame, in octets. On a
// fragment the base header and the fragment header are both IPv6 chunks, and the generic
// field reader may pick the fragment header for "ipv6.payloadLength"; this reader walks
// the chunk sequence to the base header.
inline int ipv6PayloadLength(const Packet *packet)
{
    b offset = b(0);
    while (offset < packet->getDataLength()) {
        const auto& chunk = packet->peekAt<Chunk>(offset);
        if (auto header = dynamicPtrCast<const Ipv6Header>(chunk))
            return header->getPayloadLength().get<B>();
        offset += chunk->getChunkLength();
    }
    return -1;
}

// The destination port of the UDP header that follows the fragment header of a first
// fragment. A fragment is not dissected beyond its fragment header, so no "udp.*"
// expression matches it; the header is read from the chunk sequence instead. Returns -1
// when the packet carries no fragment header.
inline int udpDestinationPortInFragment(const Packet *packet)
{
    b offset = b(0);
    while (offset < packet->getDataLength()) {
        const auto& chunk = packet->peekAt<Chunk>(offset);
        if (dynamicPtrCast<const Ipv6FragmentHeader>(chunk) != nullptr)
            return packet->peekAt<UdpHeader>(offset + chunk->getChunkLength())->getDestinationPort();
        offset += chunk->getChunkLength();
    }
    return -1;
}

} // namespace protocoltest
} // namespace inet

#endif
