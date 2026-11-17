// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/Dtls12PacketSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/transportlayer/dtls/Dtls12HelloVerifyRequest_m.h"
#include "inet/transportlayer/dtls/Dtls12CidHeader_m.h"

namespace inet::dtls {

Register_Serializer(Dtls12HelloVerifyRequest, Dtls12PacketSerializer);
Register_Serializer(Dtls12Certificate, Dtls12PacketSerializer);
Register_Serializer(Dtls12PskServerKeyExchange, Dtls12PacketSerializer);
Register_Serializer(Dtls12PskClientKeyExchange, Dtls12PacketSerializer);
Register_Serializer(Dtls12EcdheServerKeyExchange, Dtls12PacketSerializer);
Register_Serializer(Dtls12EcdheClientKeyExchange, Dtls12PacketSerializer);
Register_Serializer(Dtls12CertificateRequest, Dtls12PacketSerializer);
Register_Serializer(Dtls12ServerHelloDone, Dtls12PacketSerializer);
Register_Serializer(Dtls12ChangeCipherSpec, Dtls12PacketSerializer);
Register_Serializer(Dtls12ExplicitNonce, Dtls12PacketSerializer);
Register_Serializer(Dtls12NewSessionTicket, Dtls12PacketSerializer);
Register_Serializer(Dtls12CidHeader, Dtls12PacketSerializer);

static void requireBytes(MemoryInputStream& stream, size_t bytes)
{
    if (stream.getRemainingLength() < B(bytes))
        throw cRuntimeError("Truncated DTLS 1.2 field");
}

template<typename Getter>
static void writeVector(MemoryOutputStream& stream, size_t count, unsigned width, Getter get, bool allowEmpty = true)
{
    auto maximum = (uint32_t(1) << (8 * width)) - 1;
    if (count > maximum || (!allowEmpty && count == 0))
        throw cRuntimeError("Invalid DTLS 1.2 vector length");
    if (width == 1) stream.writeUint8(count);
    else if (width == 2) stream.writeUint16Be(count);
    else stream.writeUint24Be(count);
    for (size_t i = 0; i < count; ++i)
        stream.writeUint8(get(i));
}

template<typename Resize, typename Setter>
static void readVector(MemoryInputStream& stream, unsigned width, Resize resize, Setter set, bool allowEmpty = true)
{
    requireBytes(stream, width);
    auto count = width == 1 ? stream.readUint8() : width == 2 ? stream.readUint16Be() : stream.readUint24Be();
    requireBytes(stream, count);
    if (!allowEmpty && count == 0)
        throw cRuntimeError("Empty DTLS 1.2 vector");
    resize(count);
    for (size_t i = 0; i < count; ++i)
        set(i, stream.readUint8());
}

void Dtls12PacketSerializer::serializeFields(MemoryOutputStream& s, const Ptr<const Chunk>& chunk) const
{
    auto start = s.getLength();
    if (auto h = dynamicPtrCast<const Dtls12CidHeader>(chunk)) {
        if (h->getContentType() != 25 || h->getLegacyVersion() != 0xfefd || h->getEpoch() == 0 ||
            h->getSequenceNumber() >= (uint64_t(1) << 48) || h->getConnectionIdArraySize() == 0 || h->getConnectionIdArraySize() > 255)
            throw cRuntimeError("Invalid DTLS 1.2 CID header");
        s.writeUint8(25);
        s.writeUint16Be(h->getLegacyVersion());
        s.writeUint16Be(h->getEpoch());
        s.writeUint48Be(h->getSequenceNumber());
        for (size_t i = 0; i < h->getConnectionIdArraySize(); ++i)
            s.writeUint8(h->getConnectionId(i));
        s.writeUint16Be(h->getLength());
    }
    else if (auto h = dynamicPtrCast<const Dtls12HelloVerifyRequest>(chunk)) {
        s.writeUint16Be(h->getServerVersion());
        writeVector(s, h->getCookieArraySize(), 1, [&](size_t i) { return h->getCookie(i); });
    }
    else if (auto h = dynamicPtrCast<const Dtls12Certificate>(chunk)) {
        auto count = h->getCertificateArraySize();
        if (count > 0xfffffc)
            throw cRuntimeError("DTLS 1.2 certificate list exceeds 24 bits");
        s.writeUint24Be(count ? count + 3 : 0);
        if (count)
            writeVector(s, count, 3, [&](size_t i) { return h->getCertificate(i); });
    }
    else if (auto h = dynamicPtrCast<const Dtls12PskServerKeyExchange>(chunk))
        writeVector(s, h->getIdentityHintArraySize(), 2, [&](size_t i) { return h->getIdentityHint(i); });
    else if (auto h = dynamicPtrCast<const Dtls12PskClientKeyExchange>(chunk))
        writeVector(s, h->getIdentityArraySize(), 2, [&](size_t i) { return h->getIdentity(i); }, false);
    else if (auto h = dynamicPtrCast<const Dtls12EcdheServerKeyExchange>(chunk)) {
        s.writeUint8(3); // named_curve
        s.writeUint16Be(h->getNamedGroup());
        writeVector(s, h->getPublicKeyArraySize(), 1, [&](size_t i) { return h->getPublicKey(i); }, false);
        s.writeUint16Be(h->getSignatureScheme());
        writeVector(s, h->getSignatureArraySize(), 2, [&](size_t i) { return h->getSignature(i); }, false);
    }
    else if (auto h = dynamicPtrCast<const Dtls12EcdheClientKeyExchange>(chunk))
        writeVector(s, h->getPublicKeyArraySize(), 1, [&](size_t i) { return h->getPublicKey(i); }, false);
    else if (auto h = dynamicPtrCast<const Dtls12CertificateRequest>(chunk)) {
        writeVector(s, h->getCertificateTypesArraySize(), 1, [&](size_t i) { return h->getCertificateTypes(i); }, false);
        auto count = h->getSignatureSchemesArraySize();
        if (count == 0 || count > 32767)
            throw cRuntimeError("Invalid DTLS 1.2 signature algorithm vector");
        s.writeUint16Be(count * 2);
        for (size_t i = 0; i < count; ++i)
            s.writeUint16Be(h->getSignatureSchemes(i));
        writeVector(s, h->getCertificateAuthoritiesArraySize(), 2, [&](size_t i) { return h->getCertificateAuthorities(i); });
    }
    else if (dynamicPtrCast<const Dtls12ServerHelloDone>(chunk)) {
        // The complete body has zero bytes.
    }
    else if (auto h = dynamicPtrCast<const Dtls12ChangeCipherSpec>(chunk)) {
        if (h->getValue() != 1)
            throw cRuntimeError("Invalid DTLS 1.2 ChangeCipherSpec");
        s.writeUint8(h->getValue());
    }
    else if (auto h = dynamicPtrCast<const Dtls12NewSessionTicket>(chunk)) {
        s.writeUint32Be(h->getTicketLifetime());
        writeVector(s, h->getTicketArraySize(), 2, [&](size_t i) { return h->getTicket(i); });
    }
    else if (auto h = dynamicPtrCast<const Dtls12ExplicitNonce>(chunk))
        s.writeUint64Be(h->getValue());
    else
        throw cRuntimeError("Unknown DTLS 1.2 chunk type");
    if (s.getLength() - start != chunk->getChunkLength())
        throw cRuntimeError("DTLS 1.2 chunk length does not match its fields");
}

const Ptr<Chunk> Dtls12PacketSerializer::deserializeFields(MemoryInputStream& s, const std::type_info& typeInfo) const
{
    Ptr<Chunk> result;
    if (typeInfo == typeid(Dtls12CidHeader)) {
        auto bytes = s.getRemainingLength().get<B>();
        if (bytes < 14 || bytes > 268)
            throw cRuntimeError("Invalid DTLS 1.2 CID header size");
        auto h = makeShared<Dtls12CidHeader>();
        h->setContentType(s.readUint8());
        h->setLegacyVersion(s.readUint16Be());
        h->setEpoch(s.readUint16Be());
        h->setSequenceNumber(s.readUint48Be());
        h->setConnectionIdArraySize(bytes - 13);
        for (size_t i = 0; i < size_t(bytes - 13); ++i)
            h->setConnectionId(i, s.readUint8());
        h->setLength(s.readUint16Be());
        if (h->getContentType() != 25 || h->getLegacyVersion() != 0xfefd || h->getEpoch() == 0)
            throw cRuntimeError("Invalid DTLS 1.2 CID header fields");
        result = h;
    }
    else if (typeInfo == typeid(Dtls12NewSessionTicket)) {
        requireBytes(s, 4);
        auto h = makeShared<Dtls12NewSessionTicket>();
        h->setTicketLifetime(s.readUint32Be());
        readVector(s, 2, [&](size_t n) { h->setTicketArraySize(n); }, [&](size_t i, uint8_t v) { h->setTicket(i, v); });
        result = h;
    }
    else if (typeInfo == typeid(Dtls12HelloVerifyRequest)) {
        requireBytes(s, 2);
        auto h = makeShared<Dtls12HelloVerifyRequest>();
        h->setServerVersion(s.readUint16Be());
        readVector(s, 1, [&](size_t n) { h->setCookieArraySize(n); }, [&](size_t i, uint8_t v) { h->setCookie(i, v); });
        result = h;
    }
    else if (typeInfo == typeid(Dtls12Certificate)) {
        requireBytes(s, 3);
        auto count = s.readUint24Be();
        requireBytes(s, count);
        auto h = makeShared<Dtls12Certificate>();
        if (count) {
            requireBytes(s, 3);
            auto certificateLength = s.readUint24Be();
            if (certificateLength == 0 || certificateLength + 3 != count)
                throw cRuntimeError("Unsupported DTLS 1.2 certificate list shape");
            h->setCertificateArraySize(certificateLength);
            for (size_t i = 0; i < certificateLength; ++i)
                h->setCertificate(i, s.readUint8());
        }
        result = h;
    }
    else if (typeInfo == typeid(Dtls12PskServerKeyExchange)) {
        auto h = makeShared<Dtls12PskServerKeyExchange>();
        readVector(s, 2, [&](size_t n) { h->setIdentityHintArraySize(n); }, [&](size_t i, uint8_t v) { h->setIdentityHint(i, v); });
        result = h;
    }
    else if (typeInfo == typeid(Dtls12PskClientKeyExchange)) {
        auto h = makeShared<Dtls12PskClientKeyExchange>();
        readVector(s, 2, [&](size_t n) { h->setIdentityArraySize(n); }, [&](size_t i, uint8_t v) { h->setIdentity(i, v); }, false);
        result = h;
    }
    else if (typeInfo == typeid(Dtls12EcdheServerKeyExchange)) {
        requireBytes(s, 3);
        if (s.readUint8() != 3)
            throw cRuntimeError("Unsupported DTLS 1.2 curve type");
        auto h = makeShared<Dtls12EcdheServerKeyExchange>();
        h->setNamedGroup(s.readUint16Be());
        readVector(s, 1, [&](size_t n) { h->setPublicKeyArraySize(n); }, [&](size_t i, uint8_t v) { h->setPublicKey(i, v); }, false);
        requireBytes(s, 2);
        h->setSignatureScheme(s.readUint16Be());
        readVector(s, 2, [&](size_t n) { h->setSignatureArraySize(n); }, [&](size_t i, uint8_t v) { h->setSignature(i, v); }, false);
        result = h;
    }
    else if (typeInfo == typeid(Dtls12EcdheClientKeyExchange)) {
        auto h = makeShared<Dtls12EcdheClientKeyExchange>();
        readVector(s, 1, [&](size_t n) { h->setPublicKeyArraySize(n); }, [&](size_t i, uint8_t v) { h->setPublicKey(i, v); }, false);
        result = h;
    }
    else if (typeInfo == typeid(Dtls12CertificateRequest)) {
        auto h = makeShared<Dtls12CertificateRequest>();
        readVector(s, 1, [&](size_t n) { h->setCertificateTypesArraySize(n); }, [&](size_t i, uint8_t v) { h->setCertificateTypes(i, v); }, false);
        requireBytes(s, 2);
        auto count = s.readUint16Be();
        if (count == 0 || count % 2 != 0)
            throw cRuntimeError("Invalid DTLS 1.2 signature algorithm vector");
        requireBytes(s, count);
        h->setSignatureSchemesArraySize(count / 2);
        for (size_t i = 0; i < count / 2; ++i)
            h->setSignatureSchemes(i, s.readUint16Be());
        readVector(s, 2, [&](size_t n) { h->setCertificateAuthoritiesArraySize(n); }, [&](size_t i, uint8_t v) { h->setCertificateAuthorities(i, v); });
        result = h;
    }
    else if (typeInfo == typeid(Dtls12ServerHelloDone))
        result = makeShared<Dtls12ServerHelloDone>();
    else if (typeInfo == typeid(Dtls12ChangeCipherSpec)) {
        requireBytes(s, 1);
        auto h = makeShared<Dtls12ChangeCipherSpec>();
        h->setValue(s.readUint8());
        if (h->getValue() != 1)
            throw cRuntimeError("Invalid DTLS 1.2 ChangeCipherSpec");
        result = h;
    }
    else if (typeInfo == typeid(Dtls12ExplicitNonce)) {
        requireBytes(s, 8);
        auto h = makeShared<Dtls12ExplicitNonce>();
        h->setValue(s.readUint64Be());
        result = h;
    }
    else
        throw cRuntimeError("Unknown DTLS 1.2 chunk type");
    if (s.getRemainingLength() != b(0))
        throw cRuntimeError("Trailing DTLS 1.2 body bytes");
    return result;
}

} // namespace inet::dtls
