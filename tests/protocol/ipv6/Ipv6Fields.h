//
// Helpers for the IPv6 level 2 tests: read an address field of an observed packet, resolve
// a node's address, and read the UDP header behind a fragment header.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_IPV6FIELDS_H
#define __INET_PROTOCOLTEST_IPV6FIELDS_H

#include <functional>
#include <string>
#include <vector>

#include "PacketEvent.h"
#include "PacketField.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders_m.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {
namespace protocoltest {

// Removes the chunks in front of the first chunk of type T, hands a mutable copy of that
// chunk to fn, and puts everything back. The frame a tap holds starts with the link-layer
// header; the header under test sits behind it. IPv6 has no header checksum, so a rewrite
// of an IPv6 field needs no recompute; the UDP and ICMPv6 checksums are declared correct
// in the model's default mode, which is the simulation form of a rewriter that recomputes.
template<typename T>
inline void rewriteHeader(Packet *frame, std::function<void(T&)> fn)
{
    std::vector<Ptr<Chunk>> prefix;
    while (true) {
        const auto& front = frame->peekAtFront<Chunk>();
        if (dynamicPtrCast<const T>(front) != nullptr)
            break;
        prefix.push_back(frame->removeAtFront<Chunk>(front->getChunkLength()));
    }
    auto header = frame->removeAtFront<T>();
    fn(*header);
    frame->insertAtFront(header);
    for (auto it = prefix.rbegin(); it != prefix.rend(); ++it)
        frame->insertAtFront(*it);
}

// Shortens the payload of an Ethernet frame by n octets, keeping the frame check sequence
// at the back, and sets the IPv6 payload length to the new length of what follows the
// IPv6 header. A crafter that truncates a packet on a real path would do the same.
inline void shortenFrame(Packet *frame, B n)
{
    auto fcs = frame->removeAtBack<EthernetFcs>(B(4));
    frame->removeAtBack(n);
    frame->insertAtBack(fcs);
    // what follows the IPv6 header: the frame's data minus the chunks in front of the IPv6
    // header (a physical-layer header may precede the link-layer header at a relay), minus
    // the IPv6 header itself, minus the frame check sequence
    b offset = b(0);
    while (offset < frame->getDataLength()) {
        const auto& chunk = frame->peekAt<Chunk>(offset);
        if (dynamicPtrCast<const Ipv6Header>(chunk) != nullptr)
            break;
        offset += chunk->getChunkLength();
    }
    B afterIpv6 = B(frame->getDataLength() - offset) - B(40) - B(4);
    rewriteHeader<Ipv6Header>(frame, [afterIpv6](Ipv6Header& h) { h.setPayloadLength(afterIpv6); });
}

// True when the packet matches a PacketFilter expression; an expression that does not
// apply to the packet is a non-match, never an error.
inline bool matchesExpression(const Packet *packet, const char *expression)
{
    PacketFilter filter;
    filter.setExpression(expression);
    try {
        return filter.matches(packet);
    }
    catch (const std::exception&) {
        return false;
    }
}

// The destination port of a datagram that UDP recorded as dropped for want of a port. UDP
// removes the packet's protocol tag before it looks for a socket, so a "udp.destPort == N"
// expression cannot dissect the record; the UDP header is at the front and is read directly.
inline int udpDestinationPort(const Packet *packet)
{
    return packet->peekAtFront<UdpHeader>()->getDestinationPort();
}

// True when a datagram reaches UDP at <node>: the internet layer hands it up (kind
// SentToUpper at <node>.ipv6.ipv6) or UDP receives it (kind ReceivedFromLower at
// <node>.udp); udpExpression narrows the content, nullptr means any datagram.
inline bool deliveredToUdp(const PacketEvent& e, const char *node, const char *udpExpression)
{
    bool handoff = (e.kind == EventKind::SentToUpper && e.sourcePath == std::string(node) + ".ipv6.ipv6")
                || (e.kind == EventKind::ReceivedFromLower && e.sourcePath == std::string(node) + ".udp");
    if (!handoff)
        return false;
    return udpExpression == nullptr || matchesExpression(e.packet, udpExpression);
}

// True when an ICMPv6 error message (type below 128) leaves <node> through its interface.
inline bool icmpv6ErrorLeaves(const PacketEvent& e, const char *node)
{
    return e.kind == EventKind::SentToLower && e.sourcePath == std::string(node) + ".eth[0].mac"
        && matchesExpression(e.packet, "icmpv6.type < 128");
}

// An IPv6 address field of an observed packet, e.g. ipv6AddressOf(p, "ipv6.srcAddress").
inline const Ipv6Address *ipv6AddressOf(const Packet *packet, const char *fieldPath)
{
    return evalPacketField(packet, fieldPath).pointerValue().get<Ipv6Address>();
}

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

// The next header field of the IPv6 base header of an observed frame. A filter expression
// on "ipv6.protocolId" fails for a protocol number the dissector has no handler for; this
// reader walks the chunk sequence to the base header.
inline int ipv6NextHeader(const Packet *packet)
{
    b offset = b(0);
    while (offset < packet->getDataLength()) {
        const auto& chunk = packet->peekAt<Chunk>(offset);
        if (auto header = dynamicPtrCast<const Ipv6Header>(chunk))
            return header->getProtocolId();
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
