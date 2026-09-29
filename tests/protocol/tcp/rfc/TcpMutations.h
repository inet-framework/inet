//
// Helpers for the TCP level 3 tests: change a segment that a PacketTap holds, and compute
// the checksum that RFC 9293 defines for a segment on the wire.
//
// A change to a TCP header invalidates the checksum of the segment. Every helper here that
// changes a header therefore computes the checksum again, as a deliberate rewriter on the
// path would; otherwise the receiver would drop the segment for the checksum and never
// reach the rule under test.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_TCPMUTATIONS_H
#define __INET_PROTOCOLTEST_TCPMUTATIONS_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "PacketEvent.h"
#include "PacketField.h"
#include "inet/common/checksum/Checksum.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/transportlayer/common/TransportPseudoHeader_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader_m.h"

namespace inet {
namespace protocoltest {

// The TCP header chunk lives in its own namespace; the helpers and the tests that include
// this file name it without a qualifier.
using inet::tcp::TcpHeader;

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

// Rewrites a field of the IPv4 header and keeps the header checksum valid.
inline void rewriteIpv4Header(Packet *frame, std::function<void(Ipv4Header&)> fn)
{
    rewriteHeader<Ipv4Header>(frame, [&](Ipv4Header& header) {
        fn(header);
        header.updateChecksum();
    });
}

// The IPv4 header of a frame, read without changing it.
inline Ptr<const Ipv4Header> ipv4HeaderOf(const Packet *frame)
{
    std::unique_ptr<Packet> copy(frame->dup());
    while (true) {
        const auto& front = copy->peekAtFront<Chunk>();
        if (dynamicPtrCast<const Ipv4Header>(front) != nullptr)
            return copy->peekAtFront<Ipv4Header>();
        copy->removeAtFront<Chunk>(front->getChunkLength());
    }
}

// The checksum that RFC 9293 defines for the segment in this frame: the one's complement sum
// over a pseudo header of the two addresses, the protocol and the length, then the TCP header
// with a zero checksum field, then the data.
inline uint16_t expectedTcpChecksum(const Packet *frame)
{
    std::unique_ptr<Packet> copy(frame->dup());
    while (true) {
        const auto& front = copy->peekAtFront<Chunk>();
        if (dynamicPtrCast<const Ipv4Header>(front) != nullptr)
            break;
        copy->removeAtFront<Chunk>(front->getChunkLength());
    }
    auto ipv4 = copy->removeAtFront<Ipv4Header>();
    B segmentLength = ipv4->getTotalLengthField() - ipv4->getHeaderLength();
    auto tcp = copy->removeAtFront<TcpHeader>();
    const auto& data = copy->peekDataAt(B(0), segmentLength - tcp->getChunkLength(), Chunk::PF_ALLOW_EMPTY | Chunk::PF_ALLOW_INCORRECT);

    auto pseudoHeader = makeShared<TransportPseudoHeader>();
    pseudoHeader->setSrcAddress(ipv4->getSrcAddress());
    pseudoHeader->setDestAddress(ipv4->getDestAddress());
    pseudoHeader->setNetworkProtocolId(Protocol::ipv4.getId());
    pseudoHeader->setProtocolId(IP_PROT_TCP);
    pseudoHeader->setPacketLength(segmentLength);
    pseudoHeader->setChunkLength(B(12));

    // The header of the copy carries a zero checksum, and it is marked as computed so that it
    // can be serialized at all; a header the sender only declared correct refuses to
    // serialize. Neither change touches the frame on the wire.
    tcp->setChecksum(0);
    tcp->setChecksumMode(CHECKSUM_COMPUTED);

    MemoryOutputStream stream;
    Chunk::serialize(stream, pseudoHeader);
    Chunk::serialize(stream, tcp);
    if (data->getChunkLength() > b(0))
        Chunk::serialize(stream, data);
    return internetChecksum(stream.getData());
}

// The checksum field of the TCP header of a frame, as it stands.
inline uint16_t tcpChecksumField(const Packet *frame)
{
    std::unique_ptr<Packet> copy(frame->dup());
    while (true) {
        const auto& front = copy->peekAtFront<Chunk>();
        if (dynamicPtrCast<const TcpHeader>(front) != nullptr)
            break;
        copy->removeAtFront<Chunk>(front->getChunkLength());
    }
    return copy->peekAtFront<TcpHeader>()->getChecksum();
}

// Rewrites a field of the TCP header and computes the checksum of the segment again, so that
// the change is the only thing the receiver can object to.
inline void rewriteTcpHeader(Packet *frame, std::function<void(TcpHeader&)> fn)
{
    rewriteHeader<TcpHeader>(frame, [&](TcpHeader& header) { fn(header); });
    uint16_t checksum = expectedTcpChecksum(frame);
    rewriteHeader<TcpHeader>(frame, [checksum](TcpHeader& header) {
        header.setChecksum(checksum);
        header.setChecksumMode(CHECKSUM_COMPUTED);
    });
}

// Replaces the checksum of the TCP header with a value that is wrong for the segment, and
// changes nothing else.
inline void corruptTcpChecksum(Packet *frame)
{
    rewriteHeader<TcpHeader>(frame, [](TcpHeader& header) {
        header.setChecksum(static_cast<uint16_t>(~header.getChecksum()));
    });
}

// The sequence number of the TCP header of a frame.
inline uint32_t tcpSequenceNumberOf(const Packet *frame)
{
    std::unique_ptr<Packet> copy(frame->dup());
    while (true) {
        const auto& front = copy->peekAtFront<Chunk>();
        if (dynamicPtrCast<const TcpHeader>(front) != nullptr)
            break;
        copy->removeAtFront<Chunk>(front->getChunkLength());
    }
    return copy->peekAtFront<TcpHeader>()->getSequenceNo();
}

// Turns the segment into a reset that carries the given sequence number and no data. The
// length fields of both headers follow, so that the frame stays a well-formed datagram. Only
// the data of the segment is removed: what follows it in the frame is the trailer of the
// link layer, which must stay where it is.
inline void makeReset(Packet *frame, uint32_t sequenceNumber)
{
    const auto& ipv4 = ipv4HeaderOf(frame);
    B segmentLength = ipv4->getTotalLengthField() - ipv4->getHeaderLength();

    std::vector<Ptr<Chunk>> prefix;
    while (true) {
        const auto& front = frame->peekAtFront<Chunk>();
        if (dynamicPtrCast<const TcpHeader>(front) != nullptr)
            break;
        prefix.push_back(frame->removeAtFront<Chunk>(front->getChunkLength()));
    }
    auto header = frame->removeAtFront<TcpHeader>();
    B dataLength = segmentLength - header->getChunkLength();
    if (dataLength > B(0))
        frame->removeAtFront<Chunk>(dataLength);
    header->setRstBit(true);
    header->setSynBit(false);
    header->setFinBit(false);
    header->setPshBit(false);
    header->setSequenceNo(sequenceNumber);
    B tcpHeaderLength = header->getChunkLength();
    frame->insertAtFront(header);
    for (auto it = prefix.rbegin(); it != prefix.rend(); ++it)
        frame->insertAtFront(*it);
    rewriteIpv4Header(frame, [tcpHeaderLength](Ipv4Header& h) {
        h.setTotalLengthField(h.getHeaderLength() + tcpHeaderLength);
    });
    uint16_t checksum = expectedTcpChecksum(frame);
    rewriteHeader<TcpHeader>(frame, [checksum](TcpHeader& h) {
        h.setChecksum(checksum);
        h.setChecksumMode(CHECKSUM_COMPUTED);
    });
}

// The data length of a TCP segment, in octets: the IPv4 total length less the two header
// lengths. All three are byte-unit fields, so the subtraction is ordinary arithmetic. The
// packet must carry its IPv4 header, so this reads a frame at an interface and not a segment
// at the TCP module.
inline double tcpSegmentDataLength(const Packet *packet)
{
    return evalPacketField(packet, "ipv4.totalLengthField").doubleValueInUnit("B")
         - evalPacketField(packet, "ipv4.headerLength").doubleValueInUnit("B")
         - evalPacketField(packet, "tcp.headerLength").doubleValueInUnit("B");
}

// True when the packet matches a PacketFilter expression; an expression that does not apply
// to the packet is a non-match, never an error.
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

} // namespace protocoltest
} // namespace inet

#endif
