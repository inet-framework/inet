// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_PROTOCOLTEST_DTLS_DTLSFRAGMENTTESTUTILS_H
#define __INET_PROTOCOLTEST_DTLS_DTLSFRAGMENTTESTUTILS_H
#include "DtlsTestUtils.h"

namespace inet::protocoltest::dtls {

inline void replaceProtectedInner(Packet *packet, const Ptr<Chunk>& inner)
{
    auto oldRecord = findChunk<inet::dtls::DtlsCiphertextHeader>(packet);
    require(oldRecord != nullptr, "packed record targets protected wire data");
    int length = B(inner->getChunkLength()).get() + 16;
    int delta = length - oldRecord->getLength();
    mutateNestedHeader<EncryptedChunk>(packet, [&](auto& h) { h.setChunk(inner); h.setLength(inner->getChunkLength()); });
    mutateHeader<inet::dtls::DtlsCiphertextHeader>(packet, [=](auto& h) { h.setLength(length); });
    mutateHeader<UdpHeader>(packet, [=](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(delta)); });
    mutateHeader<Ipv4Header>(packet, [=](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(delta)); });
    resizeTapEnvelope(packet, delta);
}

inline void appendFinishedEnvelope(Packet *packet, bool malformed)
{
    auto oldHeader = findChunk<inet::dtls::DtlsHandshakeHeader>(packet);
    auto encrypted = findChunk<EncryptedChunk>(packet);
    auto oldInner = encrypted ? dynamicPtrCast<const SequenceChunk>(encrypted->getChunk()) : nullptr;
    require(oldHeader && oldHeader->getMessageType() == 8 && oldInner, "packing starts with real EncryptedExtensions");
    auto inner = makeShared<SequenceChunk>();
    for (const auto& child : oldInner->getChunks())
        if (!dynamicPtrCast<const inet::dtls::DtlsInnerContentType>(child)) inner->insertAtBack(child);
    auto header = makeShared<inet::dtls::DtlsHandshakeHeader>();
    header->setMessageType(20); header->setMessageSequence(oldHeader->getMessageSequence() + 1);
    header->setMessageLength(32); header->setFragmentLength(32); header->markImmutable();
    inner->insertAtBack(header);
    if (!malformed) {
        auto finished = makeShared<inet::tls::TlsFinished>();
        finished->setVerifyDataArraySize(32); finished->setChunkLength(B(32)); finished->markImmutable();
        inner->insertAtBack(finished);
    }
    auto type = makeShared<inet::dtls::DtlsInnerContentType>(); type->setContentType(22); type->markImmutable();
    inner->insertAtBack(type); inner->markImmutable();
    replaceProtectedInner(packet, inner);
}

// Slice the serialized body of an actual outgoing ClientHello. The enclosing
// record, UDP, IPv4 and Ethernet lengths continue to describe the resulting wire.
inline void sliceClientHello(Packet *packet, bool secondHalf, uint64_t recordNumber, bool corrupt = false)
{
    auto hello = findChunk<inet::dtls::DtlsClientHello>(packet);
    auto sequence = dynamicPtrCast<const SequenceChunk>(packet->peekData());
    require(hello && sequence, "fragmentation targets an actual typed ClientHello");
    Packet body("hello-body", hello);
    auto bytes = body.peekDataAsBytes()->getBytes();
    size_t offset = secondHalf ? bytes.size() / 2 : 0;
    size_t end = secondHalf ? bytes.size() : bytes.size() / 2;
    std::vector<uint8_t> part(bytes.begin() + offset, bytes.begin() + end);
    if (corrupt) { require(!part.empty(), "conflicting overlap has a byte"); part[0] ^= 1; }
    auto fragment = makeShared<BytesChunk>(part);
    fragment->markImmutable();
    auto replacement = makeShared<SequenceChunk>();
    bool replaced = false;
    for (const auto& child : sequence->getChunks()) {
        if (child == hello) { replacement->insertAtBack(fragment); replaced = true; }
        else replacement->insertAtBack(child);
    }
    require(replaced, "fragment replaces the complete body before the link trailer");
    int delta = (int)part.size() - (int)bytes.size();
    packet->removeData(); packet->insertAtBack(replacement);
    mutateHeader<inet::dtls::DtlsHandshakeHeader>(packet, [=](auto& h) { h.setFragmentOffset(offset); h.setFragmentLength(part.size()); });
    mutateHeader<inet::dtls::DtlsPlaintextHeader>(packet, [=](auto& h) { h.setSequenceNumber(recordNumber); h.setLength(h.getLength() + delta); });
    mutateHeader<UdpHeader>(packet, [=](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(delta)); });
    mutateHeader<Ipv4Header>(packet, [=](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(delta)); });
    resizeTapEnvelope(packet, delta);
}

// Two envelopes in ONE plaintext record, distinct from two coalesced records.
inline void packClientHello(Packet *packet, bool conflict = false, bool truncate = false)
{
    auto hello = findChunk<inet::dtls::DtlsClientHello>(packet);
    auto oldHeader = findChunk<inet::dtls::DtlsHandshakeHeader>(packet);
    auto sequence = dynamicPtrCast<const SequenceChunk>(packet->peekData());
    require(hello && oldHeader && sequence, "packing targets the real ClientHello");
    Packet body("hello-body", hello);
    auto bytes = body.peekDataAsBytes()->getBytes();
    size_t half = bytes.size() / 2;
    auto replacement = makeShared<SequenceChunk>();
    for (const auto& child : sequence->getChunks()) {
        if (child == oldHeader) continue;
        if (child != hello) { replacement->insertAtBack(child); continue; }
        for (int part = 0; part < 2; ++part) {
            size_t offset = part == 0 || conflict ? 0 : half;
            size_t count = part == 0 || conflict ? half : bytes.size() - half;
            auto h = staticPtrCast<inet::dtls::DtlsHandshakeHeader>(oldHeader->dupShared());
            h->setFragmentOffset(offset); h->setFragmentLength(count); h->markImmutable();
            replacement->insertAtBack(h);
            if (part == 1 && truncate) continue;
            std::vector<uint8_t> data(bytes.begin() + offset, bytes.begin() + offset + count);
            if (part == 1 && conflict) data[0] ^= 1;
            auto fragment = makeShared<BytesChunk>(data); fragment->markImmutable(); replacement->insertAtBack(fragment);
        }
    }
    int delta = B(replacement->getChunkLength() - packet->peekData()->getChunkLength()).get();
    packet->removeData(); packet->insertAtBack(replacement);
    mutateHeader<inet::dtls::DtlsPlaintextHeader>(packet, [=](auto& h) { h.setLength(h.getLength() + delta); });
    mutateHeader<UdpHeader>(packet, [=](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(delta)); });
    mutateHeader<Ipv4Header>(packet, [=](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(delta)); });
    resizeTapEnvelope(packet, delta);
}

}
#endif
