//
// Helpers for the UDP level 3 tests: change a datagram that a PacketTap holds, and compute
// the checksum that RFC 768 defines for a datagram on the wire.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_UDPMUTATIONS_H
#define __INET_PROTOCOLTEST_UDPMUTATIONS_H

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "PacketEvent.h"
#include "PacketField.h"
#include "inet/common/checksum/Checksum.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/transportlayer/common/TransportPseudoHeader_m.h"
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
// deliberate rewriter on the path would.
inline void rewriteIpv4Header(Packet *frame, std::function<void(Ipv4Header&)> fn)
{
    rewriteHeader<Ipv4Header>(frame, [&](Ipv4Header& header) {
        fn(header);
        header.updateChecksum();
    });
}

// Replaces the checksum of the UDP header with a value that is wrong for the datagram and
// is never zero. Zero is not a wrong checksum; it means that the sender computed none, and
// RFC 1122 §4.1.3.4 puts that case outside the discard rule.
inline void corruptUdpChecksum(Packet *frame)
{
    rewriteHeader<UdpHeader>(frame, [](UdpHeader& header) {
        uint16_t wrong = static_cast<uint16_t>(~header.getChecksum());
        if (wrong == 0x0000)
            wrong = 0x1234;
        header.setChecksum(wrong);
    });
}

// Sets the checksum of the UDP header to all zeros, which RFC 768 reads as "the transmitter
// generated no checksum".
inline void zeroUdpChecksum(Packet *frame)
{
    rewriteHeader<UdpHeader>(frame, [](UdpHeader& header) {
        header.setChecksum(0x0000);
    });
}

// Changes one octet of the data of the datagram and leaves every header field alone, the
// checksum included. The data of a generated datagram is a length without content; the
// octets it stands for are all zero. This helper replaces the data with the octets
// themselves, one of which differs, so that the change is a change on the wire and not a
// flag inside the simulation.
inline void corruptUdpData(Packet *frame)
{
    std::vector<Ptr<Chunk>> prefix;
    B dataLength = B(0);
    while (true) {
        const auto& front = frame->peekAtFront<Chunk>();
        auto udpHeader = dynamicPtrCast<const UdpHeader>(front);
        prefix.push_back(frame->removeAtFront<Chunk>(front->getChunkLength()));
        if (udpHeader != nullptr) {
            dataLength = udpHeader->getTotalLengthField() - udpHeader->getChunkLength();
            break;
        }
    }
    const auto& data = frame->removeAtFront<Chunk>(dataLength);
    MemoryOutputStream stream;
    Chunk::serialize(stream, data);
    auto octets = stream.getData();
    octets.at(0) = static_cast<uint8_t>(octets.at(0) ^ 0xFF);
    frame->insertAtFront(makeShared<BytesChunk>(octets));
    for (auto it = prefix.rbegin(); it != prefix.rend(); ++it)
        frame->insertAtFront(*it);
}

// Removes the data of the datagram and leaves the 8-octet header, which is the minimum
// length RFC 768 defines. The UDP length field, the IPv4 total length and the IPv4 header
// checksum follow the new size. The UDP checksum becomes zero, which RFC 768 reads as "the
// transmitter generated no checksum": a value that covered the old data would be wrong for
// the new datagram, and the receiver would then discard it for the checksum instead of
// answering the question this change asks.
inline void emptyUdpDatagram(Packet *frame)
{
    std::vector<Ptr<Chunk>> prefix;
    while (true) {
        const auto& front = frame->peekAtFront<Chunk>();
        if (dynamicPtrCast<const UdpHeader>(front) != nullptr)
            break;
        prefix.push_back(frame->removeAtFront<Chunk>(front->getChunkLength()));
    }
    auto header = frame->removeAtFront<UdpHeader>();
    B dataLength = header->getTotalLengthField() - header->getChunkLength();
    frame->removeAtFront<Chunk>(dataLength);
    header->setTotalLengthField(header->getChunkLength());
    header->setChecksum(0x0000);
    header->setChecksumMode(CHECKSUM_DISABLED);
    frame->insertAtFront(header);
    for (auto it = prefix.rbegin(); it != prefix.rend(); ++it)
        frame->insertAtFront(*it);
    rewriteIpv4Header(frame, [](Ipv4Header& ipv4) {
        ipv4.setTotalLengthField(ipv4.getHeaderLength() + B(8));
    });
}

// The checksum field of the UDP header of a frame, as it stands.
inline uint16_t udpChecksumField(const Packet *frame)
{
    std::unique_ptr<Packet> copy(frame->dup());
    while (true) {
        const auto& front = copy->peekAtFront<Chunk>();
        if (dynamicPtrCast<const UdpHeader>(front) != nullptr)
            break;
        copy->removeAtFront<Chunk>(front->getChunkLength());
    }
    return copy->peekAtFront<UdpHeader>()->getChecksum();
}

// The checksum that RFC 768 defines for the datagram in this frame: the one's complement of
// the one's complement sum of a pseudo header of information from the IP header, the UDP
// header and the data, with a computed zero transmitted as all ones. The value does not
// depend on what the sender put in the checksum field, which is zeroed in the copy this
// helper sums over.
inline uint16_t expectedUdpChecksum(const Packet *frame)
{
    std::unique_ptr<Packet> copy(frame->dup());
    while (true) {
        const auto& front = copy->peekAtFront<Chunk>();
        if (dynamicPtrCast<const Ipv4Header>(front) != nullptr)
            break;
        copy->removeAtFront<Chunk>(front->getChunkLength());
    }
    auto ipv4 = copy->removeAtFront<Ipv4Header>();
    auto udp = copy->removeAtFront<UdpHeader>();
    B dataLength = udp->getTotalLengthField() - udp->getChunkLength();
    const auto& data = copy->peekDataAt(B(0), dataLength, Chunk::PF_ALLOW_INCORRECT);

    auto pseudoHeader = makeShared<TransportPseudoHeader>();
    pseudoHeader->setSrcAddress(ipv4->getSrcAddress());
    pseudoHeader->setDestAddress(ipv4->getDestAddress());
    pseudoHeader->setNetworkProtocolId(Protocol::ipv4.getId());
    pseudoHeader->setProtocolId(IP_PROT_UDP);
    pseudoHeader->setPacketLength(udp->getChunkLength() + data->getChunkLength());
    pseudoHeader->setChunkLength(B(12));

    // The header of the copy carries a zero checksum, and it is marked as computed so that it
    // can be serialized at all; a header the sender only declared correct refuses to
    // serialize. Neither change touches the frame on the wire, and neither one changes the
    // value: the checksum field counts as zero in the sum by definition.
    udp->setChecksum(0x0000);
    udp->setChecksumMode(CHECKSUM_COMPUTED);

    MemoryOutputStream stream;
    Chunk::serialize(stream, pseudoHeader);
    Chunk::serialize(stream, udp);
    Chunk::serialize(stream, data);
    uint16_t checksum = internetChecksum(stream.getData());
    return checksum == 0x0000 ? 0xFFFF : checksum;
}

// True when a drop record of the UDP module belongs to a datagram that carried `dataLength`
// octets of data. A record cannot always be read by port: the module pops the UDP header
// before it records a drop for a wrong checksum (Udp::processUdpPacket), and it puts the
// header back only on the path where no program has the port. Both shapes are accepted here,
// and the length tells the datagram apart where only one is in flight.
inline bool droppedDatagramWithData(const Packet *packet, B dataLength)
{
    return packet->getDataLength() == dataLength || packet->getDataLength() == dataLength + B(8);
}

// The destination port of the UDP header of a packet whose front is the header. A drop
// record of the UDP module carries no protocol tag, so a filter expression cannot reach the
// port; this helper reads the header itself.
inline uint16_t udpDestinationPort(const Packet *packet)
{
    return packet->peekAtFront<UdpHeader>()->getDestinationPort();
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

// The two halves of a silent discard at a host, as one predicate for a single absence watch
// over the whole node: the datagram reaches the host's UDP (kind ReceivedFromLower at
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

// The addresses of every IPv4 interface of a network node, as text. The rule that a host
// sends only its own address needs the list of its own addresses, and only the node knows it.
inline std::vector<std::string> ipv4AddressesOf(cModule *node)
{
    std::vector<std::string> addresses;
    auto interfaceTable = check_and_cast<IInterfaceTable *>(node->getSubmodule("interfaceTable"));
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        auto data = interfaceTable->getInterface(i)->findProtocolData<Ipv4InterfaceData>();
        if (data != nullptr)
            addresses.push_back(data->getIPAddress().str());
    }
    return addresses;
}

// True when the address is one of the addresses of the node.
inline bool isOwnIpv4Address(cModule *node, const std::string& address)
{
    auto own = ipv4AddressesOf(node);
    return std::find(own.begin(), own.end(), address) != own.end();
}

// An IPv4 address field of an observed packet as text, e.g. ipv4Address(p, "ipv4.srcAddress").
inline std::string ipv4Address(const Packet *packet, const char *fieldPath)
{
    return evalPacketField(packet, fieldPath).pointerValue().get<Ipv4Address>()->str();
}

} // namespace protocoltest
} // namespace inet

#endif
