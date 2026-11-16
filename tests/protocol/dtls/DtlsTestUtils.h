// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_PROTOCOLTEST_DTLS_DTLSTESTUTILS_H
#define __INET_PROTOCOLTEST_DTLS_DTLSTESTUTILS_H

#include "ProtocolTest.h"
#include "DtlsTestApp.h"
#include "inet/common/packet/chunk/EncryptedChunk.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"
#include "inet/transportlayer/dtls/Dtls12CidHeader_m.h"
#include "inet/transportlayer/tls/TlsClientHello_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"

namespace inet::protocoltest::dtls {

template<typename T>
inline Ptr<const T> findChunk(const Ptr<const Chunk>& chunk)
{
    if (!chunk)
        return nullptr;
    if (auto result = dynamicPtrCast<const T>(chunk))
        return result;
    if (auto encrypted = dynamicPtrCast<const EncryptedChunk>(chunk))
        return findChunk<T>(encrypted->getChunk());
    if (auto sequence = dynamicPtrCast<const SequenceChunk>(chunk))
        for (const auto& child : sequence->getChunks())
            if (auto result = findChunk<T>(child))
                return result;
    return nullptr;
}

template<typename T>
inline Ptr<const T> findChunk(const Packet *packet)
{
    return packet ? findChunk<T>(packet->peekData()) : nullptr;
}

inline bool handshake(const MatchContext& context, int type)
{
    auto header = findChunk<inet::dtls::DtlsHandshakeHeader>(context.event.packet);
    return header && header->getMessageType() == type;
}

inline bool protectedRecord(const MatchContext& context, int epochBits)
{
    auto header = findChunk<inet::dtls::DtlsCiphertextHeader>(context.event.packet);
    return header && header->getEpochBits() == epochBits;
}

inline bool applicationRecord(const MatchContext& context)
{
    auto type = findChunk<inet::dtls::DtlsInnerContentType>(context.event.packet);
    auto legacy = findChunk<inet::dtls::DtlsPlaintextHeader>(context.event.packet);
    return type ? type->getContentType() == 23 : legacy && legacy->getContentType() == 23;
}

inline bool acknowledgement(const MatchContext& context)
{
    return findChunk<inet::dtls::DtlsAck>(context.event.packet) != nullptr;
}

inline bool packetHandshake(const Packet *packet, int type)
{
    auto h = findChunk<inet::dtls::DtlsHandshakeHeader>(packet);
    return h && h->getMessageType() == type;
}

inline bool packetApplication(const Packet *packet)
{
    auto h = findChunk<inet::dtls::DtlsInnerContentType>(packet);
    auto legacy = findChunk<inet::dtls::DtlsPlaintextHeader>(packet);
    return h ? h->getContentType() == 23 : legacy && legacy->getContentType() == 23;
}

inline bool packetAck(const Packet *packet) { return findChunk<inet::dtls::DtlsAck>(packet) != nullptr; }

inline bool packetAckForEpoch(const Packet *packet, uint64_t epoch)
{
    auto ack = findChunk<inet::dtls::DtlsAck>(packet);
    if (ack)
        for (size_t i = 0; i < ack->getEpochsArraySize(); i++)
            if (ack->getEpochs(i) == epoch)
                return true;
    return false;
}

template<typename T>
inline bool mutateHeader(Packet *packet, std::function<void(T&)> change)
{
    auto data = packet->peekData();
    std::vector<Ptr<const Chunk>> chunks;
    if (auto sequence = dynamicPtrCast<const SequenceChunk>(data))
        chunks.assign(sequence->getChunks().begin(), sequence->getChunks().end());
    else
        chunks.push_back(data);
    bool changed = false;
    for (auto& chunk : chunks)
        if (!changed && dynamicPtrCast<const T>(chunk)) {
            auto copy = staticPtrCast<T>(chunk->dupShared());
            change(*copy);
            chunk = copy;
            changed = true;
        }
    if (changed) {
        packet->removeData();
        for (const auto& chunk : chunks)
            packet->insertAtBack(chunk);
    }
    return changed;
}

// Preserve the network envelope while exercising another legal receive encoding.
inline void resizeTapEnvelope(Packet *packet, int deltaBytes)
{
    // PacketTap supplies an EthernetSignal's encapsulated Packet. OMNeT++ caches
    // the encapsulated length in that outer cPacket; changing typed chunks does
    // not update it automatically.
    if (auto outer = dynamic_cast<cPacket *>(packet->getOwner()))
        if (outer->getEncapsulatedPacket() == packet)
            outer->setBitLength(outer->getBitLength() + 8 * deltaBytes);
}

template<typename T>
inline Ptr<const Chunk> changeNestedChunk(const Ptr<const Chunk>& original, std::function<void(T&)> change, bool& changed)
{
    if (auto target = dynamicPtrCast<const T>(original)) {
        if (changed)
            return original;
        auto copy = staticPtrCast<T>(target->dupShared());
        change(*copy);
        changed = true;
        copy->markImmutable();
        return copy;
    }
    if (auto encrypted = dynamicPtrCast<const EncryptedChunk>(original)) {
        auto inner = changeNestedChunk<T>(encrypted->getChunk(), change, changed);
        auto copy = makeShared<EncryptedChunk>(constPtrCast<Chunk>(inner), encrypted->getChunkLength());
        copy->markImmutable();
        return copy;
    }
    if (auto sequence = dynamicPtrCast<const SequenceChunk>(original)) {
        auto copy = makeShared<SequenceChunk>();
        for (const auto& child : sequence->getChunks())
            copy->insertAtBack(changeNestedChunk<T>(child, change, changed));
        copy->markImmutable();
        return copy;
    }
    return original;
}

template<typename T>
inline void mutateNestedHeader(Packet *packet, std::function<void(T&)> change)
{
    bool changed = false;
    auto replacement = changeNestedChunk<T>(packet->peekData(), change, changed);
    require(changed, "fault targets an actual nested production header");
    packet->removeData();
    packet->insertAtBack(replacement);
}

inline void changeUnifiedHeader(Packet *packet, int sequenceBytes, bool lengthPresent)
{
    int delta = 1 + sequenceBytes + (lengthPresent ? 2 : 0) - 5;
    require(mutateHeader<inet::dtls::DtlsCiphertextHeader>(packet, [=](auto& h) {
        h.setSequenceNumberLength(sequenceBytes);
        h.setLengthPresent(lengthPresent);
        h.setSequenceNumber(h.getSequenceNumber() & (sequenceBytes == 1 ? 255 : 65535));
        h.setChunkLength(B(5 + delta));
    }), "alternative encoding targets a real unified header");
    require(mutateHeader<UdpHeader>(packet, [=](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(delta)); }), "frame contains UDP");
    require(mutateHeader<Ipv4Header>(packet, [=](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(delta)); }), "frame contains IPv4");
    resizeTapEnvelope(packet, delta);
}

inline void editClientHello(Packet *packet, std::function<void(inet::dtls::DtlsClientHello&)> edit)
{
    int delta = 0;
    mutateNestedHeader<inet::dtls::DtlsClientHello>(packet, [&](auto& hello) {
        auto length = [](const auto& h) { return (int)(h.getSessionIdArraySize() + h.getLegacyCookieArraySize()
            + 2 * h.getCipherSuitesArraySize() + h.getCompressionMethodsArraySize() + h.getExtensionsArraySize()); };
        int before = length(hello);
        edit(hello);
        delta = length(hello) - before;
        hello.setChunkLength(hello.getChunkLength() + B(delta));
    });
    mutateNestedHeader<inet::dtls::DtlsHandshakeHeader>(packet, [=](auto& h) {
        h.setMessageLength(h.getMessageLength() + delta);
        h.setFragmentLength(h.getFragmentLength() + delta);
    });
    require(mutateHeader<inet::dtls::DtlsPlaintextHeader>(packet, [=](auto& h) { h.setLength(h.getLength() + delta); }), "ClientHello is plaintext");
    require(mutateHeader<UdpHeader>(packet, [=](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(delta)); }), "ClientHello has UDP envelope");
    require(mutateHeader<Ipv4Header>(packet, [=](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(delta)); }), "ClientHello has IPv4 envelope");
    resizeTapEnvelope(packet, delta);
}

inline void editServerHello(Packet *packet, std::function<void(inet::tls::TlsServerHello&)> edit)
{
    int delta = 0;
    mutateNestedHeader<inet::tls::TlsServerHello>(packet, [&](auto& hello) {
        auto before = hello.getChunkLength();
        edit(hello);
        hello.setChunkLength(B(38 + hello.getSessionIdArraySize() + (hello.getExtensionsPresent() ? 2 + hello.getExtensionsArraySize() : 0)));
        delta = (hello.getChunkLength() - before).template get<B>();
    });
    mutateNestedHeader<inet::dtls::DtlsHandshakeHeader>(packet, [=](auto& h) { h.setMessageLength(h.getMessageLength() + delta); h.setFragmentLength(h.getFragmentLength() + delta); });
    require(mutateHeader<inet::dtls::DtlsPlaintextHeader>(packet, [=](auto& h) { h.setLength(h.getLength() + delta); }), "ServerHello is plaintext");
    require(mutateHeader<UdpHeader>(packet, [=](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(delta)); }), "ServerHello has UDP envelope");
    require(mutateHeader<Ipv4Header>(packet, [=](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(delta)); }), "ServerHello has IPv4 envelope");
    resizeTapEnvelope(packet, delta);
}

template<typename Hello>
inline bool hasHelloExtension(const Hello& hello, int type)
{
    for (size_t i = 0; i + 4 <= hello.getExtensionsArraySize();) {
        int current = (hello.getExtensions(i) << 8) | hello.getExtensions(i + 1);
        size_t size = (hello.getExtensions(i + 2) << 8) | hello.getExtensions(i + 3);
        if (i + 4 + size > hello.getExtensionsArraySize())
            return false;
        if (current == type)
            return true;
        i += 4 + size;
    }
    return false;
}

template<typename Hello>
inline void editHelloExtension(Hello& hello, int type, std::vector<uint8_t> replacement, bool remove = false)
{
    std::vector<uint8_t> result;
    bool found = false;
    for (size_t i = 0; i < hello.getExtensionsArraySize();) {
        require(i + 4 <= hello.getExtensionsArraySize(), "production extensions are framed");
        int current = (hello.getExtensions(i) << 8) | hello.getExtensions(i + 1);
        size_t size = (hello.getExtensions(i + 2) << 8) | hello.getExtensions(i + 3);
        require(i + 4 + size <= hello.getExtensionsArraySize(), "production extension length fits");
        if (current == type) {
            found = true;
            if (!remove) {
                result.insert(result.end(), {(uint8_t)(type >> 8), (uint8_t)type, (uint8_t)(replacement.size() >> 8), (uint8_t)replacement.size()});
                result.insert(result.end(), replacement.begin(), replacement.end());
            }
        }
        else
            for (size_t j = i; j < i + 4 + size; j++)
                result.push_back(hello.getExtensions(j));
        i += 4 + size;
    }
    require(found, "target extension was offered on the production path");
    hello.setExtensionsArraySize(result.size());
    for (size_t i = 0; i < result.size(); i++)
        hello.setExtensions(i, result[i]);
}

inline void padCertificateOffer(Packet *packet, unsigned paddingBytes)
{
    editClientHello(packet, [=](auto& hello) {
        require(!hasHelloExtension(hello, 41), "padding fixture uses certificate profile without PSK-last ordering");
        auto original = hello.getExtensionsArraySize();
        hello.setExtensionsArraySize(original + 4 + paddingBytes);
        hello.setExtensions(original, 0);
        hello.setExtensions(original + 1, 21); // RFC 7685 padding extension
        hello.setExtensions(original + 2, paddingBytes >> 8);
        hello.setExtensions(original + 3, paddingBytes & 255);
        for (size_t i = original + 4; i < hello.getExtensionsArraySize(); i++)
            hello.setExtensions(i, 0);
    });
}

inline void replaceProtectedFragment(Packet *packet, uint32_t messageLength, uint32_t offset, const std::vector<uint8_t>& bytes)
{
    auto oldHeader = findChunk<inet::dtls::DtlsHandshakeHeader>(packet);
    auto oldRecord = findChunk<inet::dtls::DtlsCiphertextHeader>(packet);
    require(oldHeader && oldRecord, "fragment fault targets a protected production handshake");
    auto header = staticPtrCast<inet::dtls::DtlsHandshakeHeader>(oldHeader->dupShared());
    header->setMessageLength(messageLength);
    header->setFragmentOffset(offset);
    header->setFragmentLength(bytes.size());
    header->markImmutable();
    auto inner = makeShared<SequenceChunk>();
    inner->insertAtBack(header);
    if (!bytes.empty()) {
        auto fragment = makeShared<BytesChunk>(bytes);
        fragment->markImmutable();
        inner->insertAtBack(fragment);
    }
    auto type = makeShared<inet::dtls::DtlsInnerContentType>();
    type->setContentType(22);
    type->markImmutable();
    inner->insertAtBack(type);
    inner->markImmutable();
    int length = B(inner->getChunkLength()).get() + 16;
    int delta = length - oldRecord->getLength();
    mutateNestedHeader<EncryptedChunk>(packet, [&](auto& h) { h.setChunk(inner); h.setLength(inner->getChunkLength()); });
    require(mutateHeader<inet::dtls::DtlsCiphertextHeader>(packet, [=](auto& h) { h.setLength(length); }), "fragment has unified record header");
    require(mutateHeader<UdpHeader>(packet, [=](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(delta)); }), "fragment has UDP envelope");
    require(mutateHeader<Ipv4Header>(packet, [=](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(delta)); }), "fragment has IPv4 envelope");
    resizeTapEnvelope(packet, delta);
}

// Keep the real network envelope and modeled tag while forcing receive-side
// deserialization from raw handshake bytes rather than a retained typed header.
inline void replaceProtectedHandshakeBytes(Packet *packet, int epochBits, uint16_t recordNumber, const std::vector<uint8_t>& bytes)
{
    auto record = findChunk<inet::dtls::DtlsCiphertextHeader>(packet);
    require(record && findChunk<EncryptedChunk>(packet), "raw handshake fault targets a protected production packet");
    auto body = makeShared<BytesChunk>(bytes);
    auto type = makeShared<inet::dtls::DtlsInnerContentType>();
    type->setContentType(22);
    body->markImmutable();
    type->markImmutable();
    auto inner = makeShared<SequenceChunk>();
    inner->insertAtBack(body);
    inner->insertAtBack(type);
    inner->markImmutable();
    int length = bytes.size() + 1 + 16;
    int delta = length - record->getLength();
    mutateNestedHeader<EncryptedChunk>(packet, [&](auto& h) { h.setChunk(inner); h.setLength(inner->getChunkLength()); });
    require(mutateHeader<inet::dtls::DtlsCiphertextHeader>(packet, [=](auto& h) { h.setEpochBits(epochBits); h.setSequenceNumber(recordNumber); h.setLength(length); }), "raw handshake retains a unified record header");
    require(mutateHeader<UdpHeader>(packet, [=](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(delta)); }), "raw handshake has UDP envelope");
    require(mutateHeader<Ipv4Header>(packet, [=](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(delta)); }), "raw handshake has IPv4 envelope");
    resizeTapEnvelope(packet, delta);
}

inline void coalesceDuplicateClientHello(Packet *packet)
{
    auto record = findChunk<inet::dtls::DtlsPlaintextHeader>(packet);
    auto header = findChunk<inet::dtls::DtlsHandshakeHeader>(packet);
    auto hello = findChunk<inet::dtls::DtlsClientHello>(packet);
    auto sequence = dynamicPtrCast<const SequenceChunk>(packet->peekData());
    require(record && header && hello && sequence, "coalescing targets a typed production ClientHello frame");
    int delta = B(record->getChunkLength() + header->getChunkLength() + hello->getChunkLength()).get();
    auto replacement = makeShared<SequenceChunk>();
    bool inserted = false;
    for (const auto& child : sequence->getChunks()) {
        replacement->insertAtBack(child);
        if (child == hello) {
            replacement->insertAtBack(record);
            replacement->insertAtBack(header);
            replacement->insertAtBack(hello);
            inserted = true;
        }
    }
    require(inserted, "duplicate record is inserted before the Ethernet trailer");
    packet->removeData();
    packet->insertAtBack(replacement);
    require(mutateHeader<UdpHeader>(packet, [=](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(delta)); }), "coalesced datagram has UDP");
    require(mutateHeader<Ipv4Header>(packet, [=](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(delta)); }), "coalesced datagram has IPv4");
    resizeTapEnvelope(packet, delta);
}

inline void appendClientHelloTail(Packet *packet)
{
    auto hello = findChunk<inet::dtls::DtlsClientHello>(packet);
    auto sequence = dynamicPtrCast<const SequenceChunk>(packet->peekData());
    require(hello && sequence, "trailing-byte fault targets a typed ClientHello");
    auto replacement = makeShared<SequenceChunk>();
    bool inserted = false;
    for (const auto& child : sequence->getChunks()) {
        replacement->insertAtBack(child);
        if (child == hello) {
            auto tail = makeShared<BytesChunk>(std::vector<uint8_t>{0});
            tail->markImmutable();
            replacement->insertAtBack(tail);
            inserted = true;
        }
    }
    require(inserted, "trailing byte is inside handshake body before Ethernet trailer");
    packet->removeData();
    packet->insertAtBack(replacement);
    mutateNestedHeader<inet::dtls::DtlsHandshakeHeader>(packet, [](auto& h) { h.setMessageLength(h.getMessageLength() + 1); h.setFragmentLength(h.getFragmentLength() + 1); });
    require(mutateHeader<inet::dtls::DtlsPlaintextHeader>(packet, [](auto& h) { h.setLength(h.getLength() + 1); }), "trailing byte is inside record");
    require(mutateHeader<UdpHeader>(packet, [](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(1)); }), "trailing byte is inside UDP");
    require(mutateHeader<Ipv4Header>(packet, [](auto& h) { h.setTotalLengthField(h.getTotalLengthField() + B(1)); }), "trailing byte is inside IPv4");
    resizeTapEnvelope(packet, 1);
}

inline Packet *rawPlaintext(int contentType, int epoch, int length)
{
    auto packet = new Packet("crafted-dtls-record");
    auto header = makeShared<inet::dtls::DtlsPlaintextHeader>();
    header->setContentType(contentType);
    header->setEpoch(epoch);
    header->setLength(length);
    packet->insertAtBack(header);
    return packet;
}

inline Packet *rawCiphertext(int epochBits)
{
    auto packet = new Packet("crafted-unknown-epoch");
    auto header = makeShared<inet::dtls::DtlsCiphertextHeader>();
    header->setEpochBits(epochBits);
    header->setLength(17);
    packet->insertAtBack(header);
    packet->insertAtBack(makeShared<BytesChunk>(std::vector<uint8_t>(17)));
    return packet;
}

inline Packet *rawFatalAlert(int description)
{
    auto packet = rawPlaintext(21, 0, 2);
    auto alert = makeShared<inet::dtls::DtlsAlert>();
    alert->setLevel(2);
    alert->setDescription(description);
    packet->insertAtBack(alert);
    return packet;
}

inline Packet *rawProtectedFinished(uint16_t messageSequence)
{
    auto header = makeShared<inet::dtls::DtlsHandshakeHeader>();
    header->setMessageType(20);
    header->setMessageLength(32);
    header->setMessageSequence(messageSequence);
    header->setFragmentLength(32);
    auto body = makeShared<inet::tls::TlsFinished>();
    body->setChunkLength(B(32));
    body->setVerifyDataArraySize(32);
    auto type = makeShared<inet::dtls::DtlsInnerContentType>();
    type->setContentType(22);
    header->markImmutable();
    body->markImmutable();
    type->markImmutable();
    auto inner = makeShared<SequenceChunk>();
    inner->insertAtBack(header);
    inner->insertAtBack(body);
    inner->insertAtBack(type);
    inner->markImmutable();
    auto record = makeShared<inet::dtls::DtlsCiphertextHeader>();
    record->setEpochBits(2);
    record->setLength(61);
    auto packet = new Packet("crafted-early-client-Finished");
    packet->insertAtBack(record);
    packet->insertAtBack(makeShared<EncryptedChunk>(inner, inner->getChunkLength()));
    packet->insertAtBack(makeShared<ByteCountChunk>(B(16)));
    return packet;
}

inline Packet *rawProtectedFatalAlert(int description)
{
    auto alert = makeShared<inet::dtls::DtlsAlert>();
    alert->setLevel(2);
    alert->setDescription(description);
    auto type = makeShared<inet::dtls::DtlsInnerContentType>();
    type->setContentType(21);
    alert->markImmutable();
    type->markImmutable();
    auto inner = makeShared<SequenceChunk>();
    inner->insertAtBack(alert);
    inner->insertAtBack(type);
    inner->markImmutable();
    auto record = makeShared<inet::dtls::DtlsCiphertextHeader>();
    record->setEpochBits(2);
    record->setLength(19);
    auto packet = new Packet("crafted-epoch-two-fatal-alert");
    packet->insertAtBack(record);
    packet->insertAtBack(makeShared<EncryptedChunk>(inner, inner->getChunkLength()));
    packet->insertAtBack(makeShared<ByteCountChunk>(B(16)));
    return packet;
}

inline void appendPlainAck(Packet *packet, uint64_t sequence)
{
    auto header = makeShared<inet::dtls::DtlsPlaintextHeader>();
    header->setContentType(26);
    header->setSequenceNumber(sequence);
    header->setLength(2);
    auto ack = makeShared<inet::dtls::DtlsAck>();
    ack->setChunkLength(B(2));
    packet->insertAtBack(header);
    packet->insertAtBack(ack);
}

inline ProtocolTest establish(const char *name)
{
    return ProtocolTest(name)
        .once(on("host1.app[0].dtls").signal("packetSentToLower")
            .filterEvent([](const MatchContext& c) { return handshake(c, 1); })
            .describe("client transmits a production ClientHello").within(1))
        .once(on("host2.app[0].driver").signal("controlStateChanged")
            .filterValue(DtlsTestDriver::ESTABLISHED)
            .describe("server processes peer Finished and establishes").within(3))
        .act(action().run<DtlsTestDriver>("host1.app[0].driver", [](DtlsTestDriver& app) {
            require(app.establishedCount == 1, "client establishes exactly once");
        }));
}

inline ProtocolTest establish12(const char *name)
{
    return ProtocolTest(name).unordered({
        on("host1.app[0].driver").signal("controlStateChanged").filterValue(DtlsTestDriver::ESTABLISHED).within(3),
        on("host2.app[0].driver").signal("controlStateChanged").filterValue(DtlsTestDriver::ESTABLISHED).within(3)
    });
}

} // namespace inet::protocoltest::dtls

#endif
