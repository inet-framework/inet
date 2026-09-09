//
// Helpers for the IPv4 level 3 tests: rewrite one header of a frame that a PacketTap
// holds, and read the header that an ICMP error message quotes.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_IPV4MUTATIONS_H
#define __INET_PROTOCOLTEST_IPV4MUTATIONS_H

#include <functional>
#include <vector>

#include "PacketEvent.h"
#include "PacketField.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {
namespace protocoltest {

// Removes the chunks in front of the first chunk of type T, hands a mutable copy of that
// chunk to fn, and puts everything back. The frame a tap holds starts with the link-layer
// header; the header under test sits behind it.
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

// Rewrites a field of the IPv4 header and keeps the header checksum valid, as any
// deliberate rewriter on the path would. The tests that craft a header use this one.
inline void rewriteIpv4Header(Packet *frame, std::function<void(Ipv4Header&)> fn)
{
    rewriteHeader<Ipv4Header>(frame, [&](Ipv4Header& header) {
        fn(header);
        header.updateChecksum();
    });
}

// Replaces the header checksum with its one's complement, a value that is wrong for every
// header, and changes nothing else. The checksum check is the one user.
inline void corruptIpv4Checksum(Packet *frame)
{
    rewriteHeader<Ipv4Header>(frame, [](Ipv4Header& header) {
        header.setChecksum(static_cast<uint16_t>(~header.getChecksum()));
    });
}

// The IPv4 header that an ICMP error message quotes. The packet must start with the ICMP
// header, as it does at the network layer of the sender and of the receiver.
inline Ptr<const Ipv4Header> quotedIpv4Header(const Packet *icmpMessage)
{
    const auto& icmp = icmpMessage->peekAtFront<IcmpHeader>();
    return icmpMessage->peekDataAt<Ipv4Header>(icmp->getChunkLength());
}

// The 8 octets after the quoted IPv4 header, read as a UDP header.
inline Ptr<const UdpHeader> quotedUdpHeader(const Packet *icmpMessage)
{
    const auto& icmp = icmpMessage->peekAtFront<IcmpHeader>();
    const auto& ipv4 = quotedIpv4Header(icmpMessage);
    return icmpMessage->peekDataAt<UdpHeader>(icmp->getChunkLength() + ipv4->getHeaderLength());
}

// True when the packet matches a PacketFilter expression; an expression that does not
// apply to the packet (e.g. icmpv4.* on a UDP datagram) is a non-match, never an error.
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

// The two halves of a silent discard at a host, as one predicate for a single absence
// watch over the whole node: the datagram reaches the host's UDP (kind ReceivedFromLower at
// <node>.udp, content matching udpExpression), or an ICMP message leaves the host (kind
// SentToLower at the host's interface, content matching icmpv4). One watch covers both,
// because two consecutive watches cannot cover the same window.
inline bool silenceBroken(const PacketEvent& e, const char *node, const char *udpExpression, EventKind udpKind = EventKind::ReceivedFromLower)
{
    std::string udpPath = std::string(node) + ".udp";
    std::string macPath = std::string(node) + ".eth[0].mac";
    if (e.kind == udpKind && e.sourcePath == udpPath)
        return udpExpression == nullptr || matchesExpression(e.packet, udpExpression);
    if (e.kind == EventKind::SentToLower && e.sourcePath == macPath)
        return matchesExpression(e.packet, "icmpv4.type >= 0");
    return false;
}

// The destination port of a datagram that UDP recorded as dropped. UDP removes the packet's
// protocol tag before it looks for a socket, so a "udp.destPort == N" expression cannot
// dissect the drop record; the UDP header is at the front and can be read directly.
inline int udpDestinationPort(const Packet *packet)
{
    return packet->peekAtFront<UdpHeader>()->getDestinationPort();
}

// An IPv4 address field of an observed packet as text, e.g. ipv4Address(p, "ipv4.srcAddress").
inline std::string ipv4Address(const Packet *packet, const char *fieldPath)
{
    return evalPacketField(packet, fieldPath).pointerValue().get<Ipv4Address>()->str();
}

} // namespace protocoltest
} // namespace inet

#endif
