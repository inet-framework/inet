//
// Helpers for the IPsec standards tests: read the IP header, the AH and ESP headers, the ESP
// trailer and the ICV out of a frame, and turn a rule into an assertion that reports what the
// packet held (doc/project/evidence/protocol/ipsec/checks.md).
//
// Why a helper header and not filter expressions. AH and ESP have no serializer, and the
// protected payload of both is an EncryptedChunk, which a filter expression does not open. The
// helpers read the frame as a sequence of chunks instead.
//
// How a protected payload is read. The checks read an ESP payload "with the keys of the SA"
// (checks.md, "Rules every check obeys"): the helpers open the EncryptedChunk of ESP and read
// the plaintext, the padding and the trailer. AH does not encrypt, and the dissector of the model
// opens the EncryptedChunk that follows an AH header; wireChunks() does the same, so the chunks
// after an AH header are in the order of the wire.
//
// How an assertion reports. A failing assertion throws with a sentence that names the rule and
// the value it found; the tester prints it as "the predicate raised: ...".
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_IPSECCHECKS_H
#define __INET_PROTOCOLTEST_IPSECCHECKS_H

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "PacketEvent.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/chunk/EncryptedChunk.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/common/packet/chunk/SliceChunk.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/networklayer/ipsec/IPsecAuthenticationHeader_m.h"
#include "inet/networklayer/ipsec/IPsecEncapsulatingSecurityPayload_m.h"
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders_m.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {
namespace protocoltest {

using ipsec::IPsecAuthenticationHeader;
using ipsec::IPsecEspHeader;
using ipsec::IPsecEspTrailer;

const int PROTO_ESP = 50;
const int PROTO_AH = 51;
const int PROTO_UDP = 17;

inline std::string num(double v) { std::ostringstream s; s << v; return s.str(); }

// For an assertion: true, or a throw that names the rule and what the packet held.
inline bool require(bool holds, const std::string& what)
{
    if (!holds)
        throw std::runtime_error(what);
    return true;
}

// The chunks of a chunk, in order: nested sequences flattened, a slice resolved to its part of
// the sliced chunk only when that part is the whole chunk, an EncryptedChunk kept as one chunk.
inline void flattenInto(const Ptr<const Chunk>& chunk, std::vector<Ptr<const Chunk>>& out)
{
    if (auto s = dynamicPtrCast<const SequenceChunk>(chunk)) {
        for (auto& e : s->getChunks())
            flattenInto(e, out);
    }
    else
        out.push_back(chunk);
}

// The chunks of a frame in the order of the wire. The payload of an AH header is opened, because
// AH does not encrypt; the payload of an ESP header stays one EncryptedChunk.
inline std::vector<Ptr<const Chunk>> wireChunks(const Packet *p)
{
    std::vector<Ptr<const Chunk>> flat, out;
    if (p == nullptr || p->getTotalLength() == b(0))
        return out;
    flattenInto(p->peekAll(), flat);
    bool afterAh = false;
    for (auto& c : flat) {
        if (afterAh) {
            if (auto e = dynamicPtrCast<const EncryptedChunk>(c)) {
                flattenInto(e->getChunk(), out);
                afterAh = false;
                continue;
            }
        }
        afterAh = dynamicPtrCast<const IPsecAuthenticationHeader>(c) != nullptr;
        out.push_back(c);
    }
    return out;
}

// The index of the first chunk of type T, or -1.
template<typename T>
inline int indexOf(const std::vector<Ptr<const Chunk>>& chunks, int from = 0)
{
    for (int i = from; i < (int)chunks.size(); i++)
        if (dynamicPtrCast<const T>(chunks[i]) != nullptr)
            return i;
    return -1;
}

template<typename T>
inline Ptr<const T> firstOf(const Packet *p)
{
    auto chunks = wireChunks(p);
    int i = indexOf<T>(chunks);
    return i < 0 ? nullptr : dynamicPtrCast<const T>(chunks[i]);
}

inline Ptr<const Ipv4Header> ipv4Of(const Packet *p) { return firstOf<Ipv4Header>(p); }
inline Ptr<const Ipv6Header> ipv6Of(const Packet *p) { return firstOf<Ipv6Header>(p); }
inline Ptr<const IPsecEspHeader> espOf(const Packet *p) { return firstOf<IPsecEspHeader>(p); }
inline Ptr<const IPsecAuthenticationHeader> ahOf(const Packet *p) { return firstOf<IPsecAuthenticationHeader>(p); }

inline bool isEsp(const Packet *p, int spi) { auto e = espOf(p); return e != nullptr && (int)e->getSpi() == spi; }
inline bool isAh(const Packet *p, int spi) { auto a = ahOf(p); return a != nullptr && (int)a->getSpi() == spi; }

// The protocol that the IP header names: the IPv4 Protocol field, or the Next Header of the
// IPv6 base header when no extension header follows it, else the Next Header of the last one.
inline int ipProtocolOf(const Packet *p)
{
    if (auto h = ipv4Of(p))
        return h->getProtocolId();
    if (auto h = ipv6Of(p))
        return h->getProtocolId();
    return -1;
}

inline int ttlOf(const Packet *p)
{
    if (auto h = ipv4Of(p))
        return h->getTimeToLive();
    if (auto h = ipv6Of(p))
        return h->getHopLimit();
    return -1;
}

// The plaintext of the ESP payload, read with the keys of the SA, as a list of chunks.
inline std::vector<Ptr<const Chunk>> espPlain(const Packet *p)
{
    std::vector<Ptr<const Chunk>> out;
    auto chunks = wireChunks(p);
    int i = indexOf<IPsecEspHeader>(chunks);
    if (i < 0 || i + 1 >= (int)chunks.size())
        return out;
    if (auto e = dynamicPtrCast<const EncryptedChunk>(chunks[i + 1]))
        flattenInto(e->getChunk(), out);
    return out;
}

// The length of the Payload Data before the plaintext: the IV of the cipher.
inline int espIvLength(const Packet *p)
{
    auto chunks = wireChunks(p);
    int i = indexOf<IPsecEspHeader>(chunks);
    if (i < 0 || i + 1 >= (int)chunks.size())
        return -1;
    auto e = dynamicPtrCast<const EncryptedChunk>(chunks[i + 1]);
    if (e == nullptr)
        return -1;
    return (int)B(e->getChunkLength() - e->getChunk()->getChunkLength()).get();
}

inline Ptr<const IPsecEspTrailer> espTrailerOf(const Packet *p)
{
    auto plain = espPlain(p);
    int i = indexOf<IPsecEspTrailer>(plain);
    return i < 0 ? nullptr : dynamicPtrCast<const IPsecEspTrailer>(plain[i]);
}

// The octets of a chunk whose contents the model holds: a byte count chunk or a bytes chunk.
inline std::vector<uint8_t> octetsOf(const Ptr<const Chunk>& c)
{
    if (auto bc = dynamicPtrCast<const ByteCountChunk>(c))
        return std::vector<uint8_t>((size_t)B(bc->getChunkLength()).get(), bc->getData());
    if (auto by = dynamicPtrCast<const BytesChunk>(c))
        return by->getBytes();
    return std::vector<uint8_t>();
}

// The padding octets of an ESP packet: the last Pad Length octets before the trailer.
inline std::vector<uint8_t> espPaddingOf(const Packet *p)
{
    std::vector<uint8_t> before;
    auto plain = espPlain(p);
    int t = indexOf<IPsecEspTrailer>(plain);
    if (t < 0)
        return before;
    for (int i = 0; i < t; i++) {
        auto o = octetsOf(plain[i]);
        if (dynamicPtrCast<const ByteCountChunk>(plain[i]) || dynamicPtrCast<const BytesChunk>(plain[i]))
            before.insert(before.end(), o.begin(), o.end());
    }
    int n = dynamicPtrCast<const IPsecEspTrailer>(plain[t])->getPadLength();
    if ((int)before.size() < n)
        return std::vector<uint8_t>();
    return std::vector<uint8_t>(before.end() - n, before.end());
}

// The octets from the start of the ESP header to the end of the Next Header field.
inline int espToTrailerEnd(const Packet *p)
{
    auto chunks = wireChunks(p);
    int i = indexOf<IPsecEspHeader>(chunks);
    if (i < 0 || i + 1 >= (int)chunks.size())
        return -1;
    return (int)B(chunks[i]->getChunkLength() + chunks[i + 1]->getChunkLength()).get();
}

inline bool isLinkChunk(const Ptr<const Chunk>& c)
{
    return dynamicPtrCast<const EthernetFcs>(c) != nullptr || dynamicPtrCast<const EthernetPadding>(c) != nullptr;
}

// The chunk right after the protected payload of an ESP packet: its ICV, or nullptr.
inline Ptr<const Chunk> espIcvOf(const Packet *p)
{
    auto chunks = wireChunks(p);
    int i = indexOf<IPsecEspHeader>(chunks);
    if (i < 0 || i + 2 >= (int)chunks.size() || isLinkChunk(chunks[i + 2]))
        return nullptr;
    return chunks[i + 2];
}

inline int espIcvLength(const Packet *p) { auto c = espIcvOf(p); return c == nullptr ? 0 : (int)B(c->getChunkLength()).get(); }

// The UDP header in a frame: in clear, after an AH header, or inside the ESP plaintext.
inline Ptr<const UdpHeader> udpOf(const Packet *p)
{
    if (espOf(p) != nullptr) {
        auto plain = espPlain(p);
        int i = indexOf<UdpHeader>(plain);
        return i < 0 ? nullptr : dynamicPtrCast<const UdpHeader>(plain[i]);
    }
    return firstOf<UdpHeader>(p);
}

// The destination port of the UDP datagram that a frame carries, protected or not, or -1.
inline int flowOf(const Packet *p)
{
    auto u = udpOf(p);
    return u == nullptr ? -1 : u->getDestinationPort();
}

inline bool isPlainUdp(const Packet *p, int port)
{
    return espOf(p) == nullptr && ahOf(p) == nullptr && flowOf(p) == port;
}

inline bool isIcmpEcho(const Packet *p, int type)
{
    auto h = firstOf<IcmpHeader>(p);
    return h != nullptr && h->getType() == type;
}

// The octets of the frame from the end of the IP header on, without the link-layer trailer.
inline int ipPayloadLength(const Packet *p)
{
    auto chunks = wireChunks(p);
    int i = indexOf<Ipv4Header>(chunks);
    if (i < 0)
        i = indexOf<Ipv6Header>(chunks);
    if (i < 0)
        return -1;
    b len = b(0);
    for (int k = i + 1; k < (int)chunks.size(); k++)
        if (!isLinkChunk(chunks[k]))
            len += chunks[k]->getChunkLength();
    return (int)B(len).get();
}

// The octets of the IP packet in a frame: the IP header and its payload.
inline int ipPacketLength(const Packet *p)
{
    auto chunks = wireChunks(p);
    int i = indexOf<Ipv4Header>(chunks);
    if (i < 0)
        i = indexOf<Ipv6Header>(chunks);
    if (i < 0)
        return -1;
    return (int)B(chunks[i]->getChunkLength()).get() + ipPayloadLength(p);
}

// A fragment: an IPv4 packet with More Fragments or an offset, or an IPv6 packet with a Fragment
// header, which the model carries as a chunk of its own after the IPv6 header.
inline Ptr<const Ipv6FragmentHeader> fragmentHeaderOf(const Packet *p) { return firstOf<Ipv6FragmentHeader>(p); }

inline bool isFragment(const Packet *p)
{
    if (auto h = ipv4Of(p))
        return h->getMoreFragments() || h->getFragmentOffset() > 0;
    return fragmentHeaderOf(p) != nullptr;
}

inline int fragmentOffsetOf(const Packet *p)
{
    if (auto h = ipv4Of(p))
        return h->getFragmentOffset();
    if (auto f = fragmentHeaderOf(p))
        return f->getFragmentOffset();
    return -1;
}

// The protocol of the fragmented packet: the IPv4 Protocol field, or the Next Header of the IPv6
// Fragment header.
inline int fragmentProtocolOf(const Packet *p)
{
    if (auto h = ipv4Of(p))
        return h->getProtocolId();
    if (auto f = fragmentHeaderOf(p))
        return f->getNextHeaderProtocol();
    return -1;
}

// Whether an AH or ESP header comes right after the IP header, or after the IPv6 Fragment header.
inline bool ipsecHeaderFirst(const Packet *p)
{
    auto ch = wireChunks(p);
    int i = indexOf<Ipv6FragmentHeader>(ch);
    if (i < 0)
        i = indexOf<Ipv4Header>(ch);
    if (i < 0)
        i = indexOf<Ipv6Header>(ch);
    if (i < 0 || i + 1 >= (int)ch.size())
        return false;
    return dynamicPtrCast<const IPsecEspHeader>(ch[i + 1]) != nullptr || dynamicPtrCast<const IPsecAuthenticationHeader>(ch[i + 1]) != nullptr;
}

inline Ptr<const Icmpv6PacketTooBigMsg> packetTooBigOf(const Packet *p) { return firstOf<Icmpv6PacketTooBigMsg>(p); }

// The destination port of a datagram that UDP passes up to an application, or -1. A datagram is
// "delivered" (checks.md) when UDP passes it up; the applications emit no signal the tester reads.
inline int deliveredPort(const Packet *p)
{
    if (p == nullptr)
        return -1;
    auto tag = p->findTag<L4PortInd>();
    return tag == nullptr ? -1 : tag->getDestPort();
}

inline std::string chunkNames(const std::vector<Ptr<const Chunk>>& chunks)
{
    std::string s;
    for (auto& c : chunks)
        s += (s.empty() ? "" : ", ") + std::string(c->getClassName()) + " " + num(B(c->getChunkLength()).get());
    return s;
}

} // namespace protocoltest
} // namespace inet

#endif
