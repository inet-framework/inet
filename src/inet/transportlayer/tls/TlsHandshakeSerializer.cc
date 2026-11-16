// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/tls/TlsHandshakeSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet::tls {

Register_Serializer(TlsClientHello, TlsHandshakeSerializer);
Register_Serializer(TlsServerHello, TlsHandshakeSerializer);
Register_Serializer(TlsEncryptedExtensions, TlsHandshakeSerializer);
Register_Serializer(TlsCertificateRequest, TlsHandshakeSerializer);
Register_Serializer(TlsCertificate, TlsHandshakeSerializer);
Register_Serializer(TlsCertificateVerify, TlsHandshakeSerializer);
Register_Serializer(TlsFinished, TlsHandshakeSerializer);
Register_Serializer(TlsKeyUpdate, TlsHandshakeSerializer);
Register_Serializer(TlsNewSessionTicket, TlsHandshakeSerializer);

static void requireBytes(MemoryInputStream& stream, size_t size)
{
    if (stream.getRemainingLength() < B(size))
        throw cRuntimeError("Truncated TLS handshake body");
}

template<typename Getter>
static void writeVector(MemoryOutputStream& stream, size_t size, unsigned lengthBytes, Getter get)
{
    // RFC 9846 Section 3 encodes vectors with a bounded length prefix.
    size_t limit = lengthBytes == 1 ? 0xff : lengthBytes == 2 ? 0xffff : 0xffffff;
    if (size > limit)
        throw cRuntimeError("TLS vector is too long");
    if (lengthBytes == 1)
        stream.writeUint8(size);
    else if (lengthBytes == 2)
        stream.writeUint16Be(size);
    else
        stream.writeUint24Be(size);
    for (size_t i = 0; i < size; ++i)
        stream.writeUint8(get(i));
}

template<typename Resize, typename Setter>
static void readVector(MemoryInputStream& stream, unsigned lengthBytes, Resize resize, Setter set)
{
    requireBytes(stream, lengthBytes);
    size_t size = lengthBytes == 1 ? stream.readUint8() : lengthBytes == 2 ? stream.readUint16Be() : stream.readUint24Be();
    requireBytes(stream, size);
    resize(size);
    for (size_t i = 0; i < size; ++i)
        set(i, stream.readUint8());
}

void TlsHandshakeSerializer::writeClientPrefix(MemoryOutputStream& s, const Ptr<const TlsClientHello>& h)
{
    if (h->getSessionIdArraySize() > 32)
        throw cRuntimeError("TLS session ID exceeds 32 bytes");
    s.writeUint16Be(h->getLegacyVersion());
    for (size_t i = 0; i < 32; i++)
        s.writeUint8(h->getRandom(i));
    s.writeUint8(h->getSessionIdArraySize());
    for (size_t i = 0; i < h->getSessionIdArraySize(); i++)
        s.writeUint8(h->getSessionId(i));
}

void TlsHandshakeSerializer::writeClientSuffix(MemoryOutputStream& s, const Ptr<const TlsClientHello>& h)
{
    auto count = h->getCipherSuitesArraySize();
    auto compressionCount = h->getCompressionMethodsArraySize();
    auto extensionsLength = h->getExtensionsArraySize();
    if (count == 0 || count > 32767 || compressionCount == 0 || compressionCount > 255 || extensionsLength > 65535)
        throw cRuntimeError("Invalid TLS ClientHello vector length");
    s.writeUint16Be(count * 2);
    for (size_t i = 0; i < count; i++)
        s.writeUint16Be(h->getCipherSuites(i));
    s.writeUint8(compressionCount);
    for (size_t i = 0; i < compressionCount; i++)
        s.writeUint8(h->getCompressionMethods(i));
    s.writeUint16Be(extensionsLength);
    for (size_t i = 0; i < extensionsLength; i++)
        s.writeUint8(h->getExtensions(i));
}

void TlsHandshakeSerializer::readClientPrefix(MemoryInputStream& s, const Ptr<TlsClientHello>& h)
{
    requireBytes(s, 35);
    h->setLegacyVersion(s.readUint16Be());
    for (size_t i = 0; i < 32; i++)
        h->setRandom(i, s.readUint8());
    auto length = s.readUint8();
    if (length > 32)
        throw cRuntimeError("TLS session ID exceeds 32 bytes");
    requireBytes(s, length);
    h->setSessionIdArraySize(length);
    for (size_t i = 0; i < length; i++)
        h->setSessionId(i, s.readUint8());
}

void TlsHandshakeSerializer::readClientSuffix(MemoryInputStream& s, const Ptr<TlsClientHello>& h)
{
    requireBytes(s, 2);
    auto length = s.readUint16Be();
    if (length < 2 || length % 2 != 0)
        throw cRuntimeError("Invalid TLS cipher suite vector");
    requireBytes(s, length + 1);
    h->setCipherSuitesArraySize(length / 2);
    for (size_t i = 0; i < length / 2; i++)
        h->setCipherSuites(i, s.readUint16Be());
    auto compressionLength = s.readUint8();
    if (compressionLength == 0)
        throw cRuntimeError("Empty TLS compression vector");
    requireBytes(s, compressionLength + 2);
    h->setCompressionMethodsArraySize(compressionLength);
    for (size_t i = 0; i < compressionLength; i++)
        h->setCompressionMethods(i, s.readUint8());
    auto extensionsLength = s.readUint16Be();
    requireBytes(s, extensionsLength);
    h->setExtensionsArraySize(extensionsLength);
    for (size_t i = 0; i < extensionsLength; i++)
        h->setExtensions(i, s.readUint8());
}

void TlsHandshakeSerializer::serializeFields(MemoryOutputStream& s, const Ptr<const Chunk>& chunk) const
{
    auto start = s.getLength();
    if (auto h = dynamicPtrCast<const TlsClientHello>(chunk)) {
        writeClientPrefix(s, h);
        writeClientSuffix(s, h);
    }
    else if (auto serverHello = dynamicPtrCast<const TlsServerHello>(chunk)) {
        if (serverHello->getSessionIdArraySize() > 32 || serverHello->getExtensionsArraySize() > 65535)
            throw cRuntimeError("Invalid TLS ServerHello vector length");
        s.writeUint16Be(serverHello->getLegacyVersion());
        for (size_t i = 0; i < 32; i++)
            s.writeUint8(serverHello->getRandom(i));
        s.writeUint8(serverHello->getSessionIdArraySize());
        for (size_t i = 0; i < serverHello->getSessionIdArraySize(); i++)
            s.writeUint8(serverHello->getSessionId(i));
        s.writeUint16Be(serverHello->getCipherSuite());
        s.writeUint8(serverHello->getCompressionMethod());
        s.writeUint16Be(serverHello->getExtensionsArraySize());
        for (size_t i = 0; i < serverHello->getExtensionsArraySize(); i++)
            s.writeUint8(serverHello->getExtensions(i));
    }
    else if (auto body = dynamicPtrCast<const TlsEncryptedExtensions>(chunk))
        writeVector(s, body->getExtensionsArraySize(), 2, [&](size_t i) { return body->getExtensions(i); });
    else if (auto body = dynamicPtrCast<const TlsCertificateRequest>(chunk)) {
        writeVector(s, body->getRequestContextArraySize(), 1, [&](size_t i) { return body->getRequestContext(i); });
        writeVector(s, body->getExtensionsArraySize(), 2, [&](size_t i) { return body->getExtensions(i); });
    }
    else if (auto body = dynamicPtrCast<const TlsCertificate>(chunk)) {
        writeVector(s, body->getRequestContextArraySize(), 1, [&](size_t i) { return body->getRequestContext(i); });
        writeVector(s, body->getCertificateListArraySize(), 3, [&](size_t i) { return body->getCertificateList(i); });
    }
    else if (auto body = dynamicPtrCast<const TlsCertificateVerify>(chunk)) {
        s.writeUint16Be(body->getSignatureScheme());
        writeVector(s, body->getSignatureArraySize(), 2, [&](size_t i) { return body->getSignature(i); });
    }
    else if (auto body = dynamicPtrCast<const TlsFinished>(chunk)) {
        for (size_t i = 0; i < body->getVerifyDataArraySize(); ++i)
            s.writeUint8(body->getVerifyData(i));
    }
    else if (auto body = dynamicPtrCast<const TlsNewSessionTicket>(chunk)) {
        if (body->getTicketArraySize() == 0)
            throw cRuntimeError("TLS session ticket is empty");
        s.writeUint32Be(body->getTicketLifetime());
        s.writeUint32Be(body->getTicketAgeAdd());
        writeVector(s, body->getTicketNonceArraySize(), 1, [&](size_t i) { return body->getTicketNonce(i); });
        writeVector(s, body->getTicketArraySize(), 2, [&](size_t i) { return body->getTicket(i); });
        writeVector(s, body->getExtensionsArraySize(), 2, [&](size_t i) { return body->getExtensions(i); });
    }
    else if (auto body = dynamicPtrCast<const TlsKeyUpdate>(chunk))
        s.writeUint8(body->getRequestUpdate());
    else
        throw cRuntimeError("Unknown TLS handshake body type");
    if (s.getLength() - start != chunk->getChunkLength())
        throw cRuntimeError("TLS handshake body length does not match its encoded fields");
}

const Ptr<Chunk> TlsHandshakeSerializer::deserializeFields(MemoryInputStream& s, const std::type_info& typeInfo) const
{
    if (typeInfo == typeid(TlsClientHello)) {
        auto h = makeShared<TlsClientHello>();
        readClientPrefix(s, h);
        readClientSuffix(s, h);
        return h;
    }
    if (typeInfo == typeid(TlsNewSessionTicket)) {
        requireBytes(s, 8);
        auto body = makeShared<TlsNewSessionTicket>();
        body->setTicketLifetime(s.readUint32Be());
        body->setTicketAgeAdd(s.readUint32Be());
        readVector(s, 1, [&](size_t n) { body->setTicketNonceArraySize(n); }, [&](size_t i, uint8_t v) { body->setTicketNonce(i, v); });
        readVector(s, 2, [&](size_t n) { body->setTicketArraySize(n); }, [&](size_t i, uint8_t v) { body->setTicket(i, v); });
        readVector(s, 2, [&](size_t n) { body->setExtensionsArraySize(n); }, [&](size_t i, uint8_t v) { body->setExtensions(i, v); });
        if (body->getTicketArraySize() == 0)
            throw cRuntimeError("TLS session ticket is empty");
        return body;
    }
    if (typeInfo == typeid(TlsEncryptedExtensions)) {
        auto body = makeShared<TlsEncryptedExtensions>();
        readVector(s, 2, [&](size_t n) { body->setExtensionsArraySize(n); }, [&](size_t i, uint8_t v) { body->setExtensions(i, v); });
        return body;
    }
    if (typeInfo == typeid(TlsCertificateRequest)) {
        auto body = makeShared<TlsCertificateRequest>();
        readVector(s, 1, [&](size_t n) { body->setRequestContextArraySize(n); }, [&](size_t i, uint8_t v) { body->setRequestContext(i, v); });
        readVector(s, 2, [&](size_t n) { body->setExtensionsArraySize(n); }, [&](size_t i, uint8_t v) { body->setExtensions(i, v); });
        return body;
    }
    if (typeInfo == typeid(TlsCertificate)) {
        auto body = makeShared<TlsCertificate>();
        readVector(s, 1, [&](size_t n) { body->setRequestContextArraySize(n); }, [&](size_t i, uint8_t v) { body->setRequestContext(i, v); });
        readVector(s, 3, [&](size_t n) { body->setCertificateListArraySize(n); }, [&](size_t i, uint8_t v) { body->setCertificateList(i, v); });
        return body;
    }
    if (typeInfo == typeid(TlsCertificateVerify)) {
        requireBytes(s, 2);
        auto body = makeShared<TlsCertificateVerify>();
        body->setSignatureScheme(s.readUint16Be());
        readVector(s, 2, [&](size_t n) { body->setSignatureArraySize(n); }, [&](size_t i, uint8_t v) { body->setSignature(i, v); });
        return body;
    }
    if (typeInfo == typeid(TlsFinished)) {
        auto body = makeShared<TlsFinished>();
        auto size = B(s.getRemainingLength()).get();
        body->setVerifyDataArraySize(size);
        for (size_t i = 0; i < size; ++i)
            body->setVerifyData(i, s.readUint8());
        return body;
    }
    if (typeInfo == typeid(TlsKeyUpdate)) {
        requireBytes(s, 1);
        auto body = makeShared<TlsKeyUpdate>();
        body->setRequestUpdate(s.readUint8());
        return body;
    }
    if (typeInfo != typeid(TlsServerHello))
        throw cRuntimeError("Unknown TLS handshake body type");
    requireBytes(s, 35);
    auto h = makeShared<TlsServerHello>();
    h->setLegacyVersion(s.readUint16Be());
    for (size_t i = 0; i < 32; i++)
        h->setRandom(i, s.readUint8());
    auto length = s.readUint8();
    if (length > 32)
        throw cRuntimeError("TLS session ID exceeds 32 bytes");
    requireBytes(s, length + 5);
    h->setSessionIdArraySize(length);
    for (size_t i = 0; i < length; i++)
        h->setSessionId(i, s.readUint8());
    h->setCipherSuite(s.readUint16Be());
    h->setCompressionMethod(s.readUint8());
    auto extensionsLength = s.readUint16Be();
    requireBytes(s, extensionsLength);
    h->setExtensionsArraySize(extensionsLength);
    for (size_t i = 0; i < extensionsLength; i++)
        h->setExtensions(i, s.readUint8());
    return h;
}

} // namespace inet::tls
