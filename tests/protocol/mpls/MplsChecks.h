//
// Helpers for the MPLS standards tests: read the label stack, the PPP and Ethernet headers, the
// IPv4 header and the UDP port out of a frame, and turn a rule into an assertion that reports
// what the frame held (doc/project/evidence/protocol/mpls/checks.md).
//
// How an entry is read. The checks read the fields of a label stack entry "from its 4 octets, in
// the order of transmission" (checks.md, "Rules every check obeys"). The helpers serialize each
// MplsHeader chunk and decode the octets, so a field that the serializer writes in the wrong
// place fails a check, and a field that a chunk holds but the wire does not carry never passes
// one.
//
// How a TTL of one datagram is compared on two links. A guard on the first link records the TTL
// of each datagram by its IPv4 Identification and never matches; a guard on the second link reads
// the record. The record advances on every event of the first link.
//
// How an assertion reports. A failing assertion throws with a sentence that names the rule and
// the value it found; the tester prints it as "the predicate raised: ...".
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_MPLSCHECKS_H
#define __INET_PROTOCOLTEST_MPLSCHECKS_H

#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "PacketEvent.h"
#include "inet/common/MemoryOutputStream.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/linklayer/ppp/PppFrame_m.h"
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/mpls/MplsPacket_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {
namespace protocoltest {

const int PPP_IPV4 = 0x0021;
const int PPP_MPLS_UNICAST = 0x0281;
const int PPP_MPLSCP = 0x8281;
const int MPLS_ETHERTYPE_UNICAST = 0x8847;

inline std::string num(double v) { std::ostringstream s; s << v; return s.str(); }

// For an assertion: true, or a throw that names the rule and what the frame held.
inline bool require(bool holds, const std::string& what)
{
    if (!holds)
        throw std::runtime_error(what);
    return true;
}

// The chunks of a frame in order, with nested sequences flattened.
inline void flattenInto(const Ptr<const Chunk>& chunk, std::vector<Ptr<const Chunk>>& out)
{
    if (auto s = dynamicPtrCast<const SequenceChunk>(chunk)) {
        for (auto& e : s->getChunks())
            flattenInto(e, out);
    }
    else
        out.push_back(chunk);
}

inline std::vector<Ptr<const Chunk>> chunksOf(const Packet *p)
{
    std::vector<Ptr<const Chunk>> out;
    if (p != nullptr && p->getTotalLength() > b(0))
        flattenInto(p->peekAll(), out);
    return out;
}

template<typename T>
inline Ptr<const T> firstOf(const Packet *p)
{
    for (auto& c : chunksOf(p))
        if (auto t = dynamicPtrCast<const T>(c))
            return t;
    return nullptr;
}

// ------------------------------------------------------------------ the label stack

// One label stack entry, decoded from its 4 octets (RFC 3032 §2.1, RFC 5462 §2.1).
struct LabelEntry
{
    std::vector<uint8_t> octets;
    uint32_t label = 0;
    int tc = 0;
    bool s = false;
    int ttl = 0;
};

inline LabelEntry decodeEntry(const Ptr<const Chunk>& chunk)
{
    MemoryOutputStream stream;
    Chunk::serialize(stream, chunk);
    LabelEntry e;
    e.octets = stream.getData();
    if (e.octets.size() == 4) {
        const auto& o = e.octets;
        e.label = ((uint32_t)o[0] << 12) | ((uint32_t)o[1] << 4) | (o[2] >> 4);
        e.tc = (o[2] >> 1) & 0x7;
        e.s = (o[2] & 0x1) != 0;
        e.ttl = o[3];
    }
    return e;
}

// The label stack of a frame, top entry first: the MplsHeader chunks in the order of the frame.
inline std::vector<LabelEntry> labelStack(const Packet *p)
{
    std::vector<LabelEntry> out;
    for (auto& c : chunksOf(p))
        if (dynamicPtrCast<const MplsHeader>(c) != nullptr)
            out.push_back(decodeEntry(c));
    return out;
}

inline bool isLabeled(const Packet *p) { return firstOf<MplsHeader>(p) != nullptr; }

inline int topLabel(const Packet *p) { auto s = labelStack(p); return s.empty() ? -1 : (int)s.front().label; }

inline int topTtl(const Packet *p) { auto s = labelStack(p); return s.empty() ? -1 : s.front().ttl; }

inline std::string stackStr(const Packet *p)
{
    auto stack = labelStack(p);
    if (stack.empty())
        return "no label stack entry";
    std::ostringstream o;
    for (size_t i = 0; i < stack.size(); i++) {
        auto& e = stack[i];
        o << (i ? ", " : "") << "label " << e.label << " TC " << e.tc << " S " << e.s << " TTL " << e.ttl << " (octets";
        for (auto x : e.octets)
            o << " " << std::hex << (x < 16 ? "0" : "") << (int)x << std::dec;
        o << ")";
    }
    return o.str();
}

// The chunk names of a frame, for a message.
inline std::string chunkNames(const Packet *p)
{
    std::string s;
    for (auto& c : chunksOf(p))
        s += (s.empty() ? "" : " ") + std::string(c->getClassName());
    return s;
}

// ------------------------------------------------------------------ the link headers

inline int pppProtocol(const Packet *p) { auto h = firstOf<PppHeader>(p); return h == nullptr ? -1 : h->getProtocol(); }

inline int etherType(const Packet *p) { auto h = firstOf<EthernetMacHeader>(p); return h == nullptr ? -1 : h->getTypeOrLength(); }

inline bool isLinkChunk(const Ptr<const Chunk>& c)
{
    return dynamicPtrCast<const PppHeader>(c) != nullptr || dynamicPtrCast<const PppTrailer>(c) != nullptr
        || dynamicPtrCast<const EthernetMacHeader>(c) != nullptr || dynamicPtrCast<const EthernetPadding>(c) != nullptr
        || dynamicPtrCast<const EthernetFcs>(c) != nullptr;
}

// The length of what the link carries: every chunk except the link header, the padding and the
// trailer. On a PPP link this is the Information field.
inline int payloadLength(const Packet *p)
{
    b len = b(0);
    for (auto& c : chunksOf(p))
        if (!isLinkChunk(c))
            len += c->getChunkLength();
    return (int)B(len).get();
}

// True when the frame is: link header, label stack entries, the IPv4 header right after the entry
// with the S bit, and nothing else before the payload of the IPv4 datagram.
inline bool stackBetweenLinkAndIpv4(const Packet *p)
{
    auto chunks = chunksOf(p);
    size_t i = 0;
    if (i < chunks.size() && isLinkChunk(chunks[i]))
        i++;
    size_t first = i;
    while (i < chunks.size() && dynamicPtrCast<const MplsHeader>(chunks[i]) != nullptr) {
        auto e = decodeEntry(chunks[i]);
        i++;
        if (e.s)
            break;
    }
    return i > first && i < chunks.size() && dynamicPtrCast<const Ipv4Header>(chunks[i]) != nullptr;
}

// ------------------------------------------------------------------ the IPv4 datagram

inline Ptr<const Ipv4Header> ipv4Of(const Packet *p) { return firstOf<Ipv4Header>(p); }

inline int ipTtl(const Packet *p) { auto h = ipv4Of(p); return h == nullptr ? -1 : h->getTimeToLive(); }

inline int ipId(const Packet *p) { auto h = ipv4Of(p); return h == nullptr ? -1 : h->getIdentification(); }

inline bool isFragment(const Packet *p)
{
    auto h = ipv4Of(p);
    return h != nullptr && (h->getMoreFragments() || h->getFragmentOffset() != 0);
}

// The flow of a datagram: its UDP destination port, or -1 when the frame has no UDP header, as a
// fragment after the first one.
inline int flowOf(const Packet *p)
{
    if (ipv4Of(p) == nullptr)
        return -1;
    auto u = firstOf<UdpHeader>(p);
    return u == nullptr ? -1 : u->getDestinationPort();
}

// The port on which UDP passes a datagram up to an application.
inline int deliveredPort(const Packet *p)
{
    if (p == nullptr)
        return -1;
    auto tag = p->findTag<L4PortInd>();
    return tag == nullptr ? -1 : tag->getDestPort();
}

inline Ptr<const IcmpPtb> fragmentationNeededOf(const Packet *p) { return firstOf<IcmpPtb>(p); }

// ------------------------------------------------------------------ records across links

// The TTL that each datagram carried on one link, by its IPv4 Identification.
inline std::map<int, int>& ttlRecord(const std::string& link)
{
    static std::map<std::string, std::map<int, int>> records;
    return records[link];
}

// For a guard that only records: stores the value and never matches.
inline bool recordTtl(const std::string& link, const Packet *p, int ttl)
{
    if (ipId(p) >= 0)
        ttlRecord(link)[ipId(p)] = ttl;
    return false;
}

// The recorded TTL of the same datagram on the link, or -1000 when that link carried no such
// datagram.
inline int recordedTtl(const std::string& link, const Packet *p)
{
    auto& r = ttlRecord(link);
    auto it = r.find(ipId(p));
    return it == r.end() ? -1000 : it->second;
}

// True once a frame of the PPP protocol went by, for a rule about what comes before what.
inline bool& seenProtocol(int protocol)
{
    static std::map<int, bool> seen;
    return seen[protocol];
}

} // namespace protocoltest
} // namespace inet

#endif
