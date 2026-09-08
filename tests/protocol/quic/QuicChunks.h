//
// QUIC conformance suite -- shared test support.
//
// QUIC has no protocol dissector registered in this INET tree: Quic.cc never tags an
// outgoing packet with a PacketProtocolTag naming Protocol::quic, and even if it did, no
// ProtocolDissector is registered for that protocol (confirmed by grepping
// src/inet/common/packet/dissector/ and src/inet/transportlayer/quic/ -- there is no
// Register_Protocol_Dissector for Protocol::quic anywhere). PacketDissector.cc's
// doDissectPacket() falls back to DefaultProtocolDissector for any protocol it has no
// registered dissector for, and that dissector (ProtocolDissector.cc) makes exactly one
// visitChunk(packet->peekData(), protocol) call for the whole remainder -- so the class
// map PacketField.cc's evalPacketField() builds only ever gets one entry for everything
// past the UDP header ("SequenceChunk"), never the concrete QUIC classes. This was
// confirmed empirically with a throwaway probe test (see the report): every
// evalPacketField(pkt, "SomeQuicClass.field") call threw "protocol '...' not present in
// packet", at every observation point tried.
//
// What *does* work, also confirmed by the same probe: walking the packet's own chunk
// tree directly. inet::quic::QuicPacket::createOmnetPacket() (PacketBuilder.cc's
// counterpart, packet/QuicPacket.cc) builds each QUIC packet with
// pkt->insertAtBack(header) then, per frame, insertAtBack(frameHeader) and
// insertAtBack(frameData) if the frame carries data -- concatenating several distinct
// FieldsChunk-derived C++ types. INET cannot merge chunks of different concrete types, so
// this concatenation becomes a SequenceChunk; when UDP later prepends its own UdpHeader,
// INET flattens rather than nests it, so the packet's whole content -- UDP header
// included, when the observation point precedes UDP popping it -- is one flat,
// wire-ordered SequenceChunk.
// quicChunks()/findQuicChunk<T>() below walk that list directly, bypassing
// PacketFilter/evalPacketField entirely.
//
// Usage from a test:
//
//   if (auto sf = findQuicChunk<StreamFrameHeader>(c.event.packet))
//       ... sf->getStreamId(), sf->getOffset(), sf->getLength() ...
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
#ifndef __INET_PROTOCOLTEST_QUIC_QUICCHUNKS_H
#define __INET_PROTOCOLTEST_QUIC_QUICCHUNKS_H

#include "ProtocolTest.h"

#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/transportlayer/quic/packet/PacketHeader_m.h"
#include "inet/transportlayer/quic/packet/FrameHeader_m.h"

namespace inet {
namespace protocoltest {

using namespace inet::quic;

// Every chunk making up `pkt`'s current data region, in wire order: the UDP header first
// if the observation point precedes UDP popping it, then the QUIC packet header, then
// each frame's header (and its data chunk, if it carries one).
inline std::vector<Ptr<const Chunk>> quicChunks(const Packet *pkt)
{
    std::vector<Ptr<const Chunk>> result;
    if (pkt == nullptr)
        return result;
    Ptr<const Chunk> content = pkt->peekData();
    if (content == nullptr)
        return result;
    if (auto seq = dynamicPtrCast<const SequenceChunk>(content)) {
        for (const auto& kid : seq->getChunks())
            result.push_back(kid);
    }
    else
        result.push_back(content);
    return result;
}

// The first chunk of exact dynamic type T among pkt's chunks (e.g.
// findQuicChunk<StreamFrameHeader>(pkt)), or nullptr if none is present.
template<typename T>
inline Ptr<const T> findQuicChunk(const Packet *pkt)
{
    for (const auto& kid : quicChunks(pkt)) {
        if (auto t = dynamicPtrCast<const T>(kid))
            return t;
    }
    return nullptr;
}

// Every chunk of exact dynamic type T among pkt's chunks, in wire order. A QUIC packet
// carries several frames, so a check that must hold for *every* frame of a kind -- for
// example, every stream identifier an endpoint sends on -- has to look at all of them.
// findQuicChunk<T>() returns only the first, which is enough to answer "is there one?"
// and wrong for "do they all ...?".
template<typename T>
inline std::vector<Ptr<const T>> findAllQuicChunks(const Packet *pkt)
{
    std::vector<Ptr<const T>> result;
    for (const auto& kid : quicChunks(pkt)) {
        if (auto t = dynamicPtrCast<const T>(kid))
            result.push_back(t);
    }
    return result;
}

} // namespace protocoltest
} // namespace inet

#endif
