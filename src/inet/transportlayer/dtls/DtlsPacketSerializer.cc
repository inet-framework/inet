// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/DtlsPacketSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"
#include "inet/transportlayer/tls/TlsHandshakeSerializer.h"

namespace inet::dtls {

Register_Serializer(DtlsPlaintextHeader, DtlsPacketSerializer);
Register_Serializer(DtlsCiphertextHeader, DtlsPacketSerializer);
Register_Serializer(DtlsHandshakeHeader, DtlsPacketSerializer);
Register_Serializer(DtlsClientHello, DtlsPacketSerializer);
Register_Serializer(DtlsInnerContentType, DtlsPacketSerializer);
Register_Serializer(DtlsAck, DtlsPacketSerializer);
Register_Serializer(DtlsAlert, DtlsPacketSerializer);
Register_Serializer(DtlsNewConnectionId, DtlsPacketSerializer);
Register_Serializer(DtlsRequestConnectionId, DtlsPacketSerializer);
Register_Serializer(DtlsHeartbeat, DtlsPacketSerializer);

static void requireBytes(MemoryInputStream& stream, size_t size)
{
    if (stream.getRemainingLength() < B(size))
        throw cRuntimeError("Truncated DTLS field");
}

void DtlsPacketSerializer::serializeFields(MemoryOutputStream& s, const Ptr<const Chunk>& chunk) const
{
    // RFC 9147 Sections 4.2, 5.2, and 7 define these record and handshake fields.
    auto start = s.getLength();
    if (auto h = dynamicPtrCast<const DtlsPlaintextHeader>(chunk)) {
        if (h->getSequenceNumber() >= (uint64_t(1) << 48))
            throw cRuntimeError("DTLS plaintext sequence number exceeds 48 bits");
        s.writeUint8(h->getContentType());
        s.writeUint16Be(h->getLegacyVersion());
        s.writeUint16Be(h->getEpoch());
        s.writeUint48Be(h->getSequenceNumber());
        s.writeUint16Be(h->getLength());
    }
    else if (auto h = dynamicPtrCast<const DtlsCiphertextHeader>(chunk)) {
        auto sequenceLength = h->getSequenceNumberLength();
        if (h->getEpochBits() > 3 || (sequenceLength != 1 && sequenceLength != 2) || (sequenceLength == 1 && h->getSequenceNumber() > 255))
            throw cRuntimeError("Invalid DTLS unified header fields");
        if (h->getConnectionIdArraySize() > 255)
            throw cRuntimeError("DTLS connection ID exceeds 255 bytes");
        s.writeUint8(0x20 | (h->getConnectionIdArraySize() ? 0x10 : 0) | (sequenceLength == 2 ? 0x08 : 0) | (h->getLengthPresent() ? 0x04 : 0) | h->getEpochBits());
        for (size_t i = 0; i < h->getConnectionIdArraySize(); ++i)
            s.writeUint8(h->getConnectionId(i));
        if (sequenceLength == 2)
            s.writeUint16Be(h->getSequenceNumber());
        else
            s.writeUint8(h->getSequenceNumber());
        if (h->getLengthPresent())
            s.writeUint16Be(h->getLength());
    }
    else if (auto h = dynamicPtrCast<const DtlsHandshakeHeader>(chunk)) {
        if (h->getMessageLength() > 0xffffff || h->getFragmentOffset() > h->getMessageLength() || h->getFragmentLength() > h->getMessageLength() - h->getFragmentOffset())
            throw cRuntimeError("Invalid DTLS handshake fragment bounds");
        s.writeUint8(h->getMessageType());
        s.writeUint24Be(h->getMessageLength());
        s.writeUint16Be(h->getMessageSequence());
        s.writeUint24Be(h->getFragmentOffset());
        s.writeUint24Be(h->getFragmentLength());
    }
    else if (auto h = dynamicPtrCast<const DtlsClientHello>(chunk)) {
        if (h->getLegacyCookieArraySize() > 255)
            throw cRuntimeError("DTLS legacy cookie exceeds 255 bytes");
        tls::TlsHandshakeSerializer::writeClientPrefix(s, h);
        s.writeUint8(h->getLegacyCookieArraySize());
        for (size_t i = 0; i < h->getLegacyCookieArraySize(); i++)
            s.writeUint8(h->getLegacyCookie(i));
        tls::TlsHandshakeSerializer::writeClientSuffix(s, h);
    }
    else if (auto h = dynamicPtrCast<const DtlsInnerContentType>(chunk))
        s.writeUint8(h->getContentType());
    else if (auto ack = dynamicPtrCast<const DtlsAck>(chunk)) {
        auto count = ack->getEpochsArraySize();
        if (count != ack->getSequenceNumbersArraySize() || count > 4095)
            throw cRuntimeError("Invalid DTLS ACK record list");
        s.writeUint16Be(count * 16);
        for (size_t i = 0; i < count; i++) {
            s.writeUint64Be(ack->getEpochs(i));
            s.writeUint64Be(ack->getSequenceNumbers(i));
        }
    }
    else if (auto cid = dynamicPtrCast<const DtlsNewConnectionId>(chunk)) {
        if (cid->getConnectionIdsArraySize() > 65535)
            throw cRuntimeError("DTLS CID list exceeds 65535 bytes");
        s.writeUint16Be(cid->getConnectionIdsArraySize());
        for (size_t i = 0; i < cid->getConnectionIdsArraySize(); ++i)
            s.writeUint8(cid->getConnectionIds(i));
        s.writeUint8(cid->getUsage());
    }
    else if (auto request = dynamicPtrCast<const DtlsRequestConnectionId>(chunk))
        s.writeUint8(request->getNumCids());
    else if (auto heartbeat = dynamicPtrCast<const DtlsHeartbeat>(chunk)) {
        if (heartbeat->getPaddingArraySize() < 16 || heartbeat->getPayloadArraySize() + heartbeat->getPaddingArraySize() + 3 > 16384)
            throw cRuntimeError("Invalid DTLS heartbeat size or padding");
        s.writeUint8(heartbeat->getType());
        s.writeUint16Be(heartbeat->getPayloadArraySize());
        for (size_t i = 0; i < heartbeat->getPayloadArraySize(); ++i)
            s.writeUint8(heartbeat->getPayload(i));
        for (size_t i = 0; i < heartbeat->getPaddingArraySize(); ++i)
            s.writeUint8(heartbeat->getPadding(i));
    }
    else if (auto alert = dynamicPtrCast<const DtlsAlert>(chunk)) {
        s.writeUint8(alert->getLevel());
        s.writeUint8(alert->getDescription());
    }
    else
        throw cRuntimeError("Unknown DTLS chunk type");
    if (s.getLength() - start != chunk->getChunkLength())
        throw cRuntimeError("DTLS chunk length does not match its encoded fields");
}

const Ptr<Chunk> DtlsPacketSerializer::deserializeFields(MemoryInputStream& s, const std::type_info& typeInfo) const
{
    if (typeInfo == typeid(DtlsPlaintextHeader)) {
        requireBytes(s, 13);
        auto h = makeShared<DtlsPlaintextHeader>();
        h->setContentType(s.readUint8());
        h->setLegacyVersion(s.readUint16Be());
        h->setEpoch(s.readUint16Be());
        h->setSequenceNumber(s.readUint48Be());
        h->setLength(s.readUint16Be());
        return h;
    }
    if (typeInfo == typeid(DtlsCiphertextHeader)) {
        requireBytes(s, 1);
        auto first = s.readUint8();
        if ((first & 0xe0) != 0x20)
            throw cRuntimeError("Unsupported DTLS unified header");
        auto h = makeShared<DtlsCiphertextHeader>();
        h->setEpochBits(first & 3);
        h->setSequenceNumberLength((first & 8) ? 2 : 1);
        h->setLengthPresent(first & 4);
        // This model's receive endpoints allocate fixed eight-byte CIDs.
        // CID length is negotiated out of band and is not a record field.
        if (first & 0x10) {
            requireBytes(s, 8);
            h->setConnectionIdArraySize(8);
            for (size_t i = 0; i < 8; ++i)
                h->setConnectionId(i, s.readUint8());
        }
        requireBytes(s, h->getSequenceNumberLength() + (h->getLengthPresent() ? 2 : 0));
        h->setSequenceNumber(h->getSequenceNumberLength() == 2 ? s.readUint16Be() : s.readUint8());
        if (h->getLengthPresent())
            h->setLength(s.readUint16Be());
        return h;
    }
    if (typeInfo == typeid(DtlsHandshakeHeader)) {
        requireBytes(s, 12);
        auto h = makeShared<DtlsHandshakeHeader>();
        h->setMessageType(s.readUint8());
        h->setMessageLength(s.readUint24Be());
        h->setMessageSequence(s.readUint16Be());
        h->setFragmentOffset(s.readUint24Be());
        h->setFragmentLength(s.readUint24Be());
        if (h->getFragmentOffset() > h->getMessageLength() || h->getFragmentLength() > h->getMessageLength() - h->getFragmentOffset())
            throw cRuntimeError("Invalid DTLS handshake fragment bounds");
        return h;
    }
    if (typeInfo == typeid(DtlsClientHello)) {
        auto h = makeShared<DtlsClientHello>();
        tls::TlsHandshakeSerializer::readClientPrefix(s, h);
        requireBytes(s, 1);
        auto length = s.readUint8();
        requireBytes(s, length);
        h->setLegacyCookieArraySize(length);
        for (size_t i = 0; i < length; i++)
            h->setLegacyCookie(i, s.readUint8());
        tls::TlsHandshakeSerializer::readClientSuffix(s, h);
        return h;
    }
    if (typeInfo == typeid(DtlsInnerContentType)) {
        requireBytes(s, 1);
        auto h = makeShared<DtlsInnerContentType>();
        h->setContentType(s.readUint8());
        return h;
    }
    if (typeInfo == typeid(DtlsAck)) {
        requireBytes(s, 2);
        auto length = s.readUint16Be();
        if (length % 16 != 0)
            throw cRuntimeError("Invalid DTLS ACK length");
        requireBytes(s, length);
        auto ack = makeShared<DtlsAck>();
        ack->setEpochsArraySize(length / 16);
        ack->setSequenceNumbersArraySize(length / 16);
        for (size_t i = 0; i < length / 16; i++) {
            ack->setEpochs(i, s.readUint64Be());
            ack->setSequenceNumbers(i, s.readUint64Be());
        }
        return ack;
    }
    if (typeInfo == typeid(DtlsNewConnectionId)) {
        requireBytes(s, 2);
        auto length = s.readUint16Be();
        requireBytes(s, length + 1);
        auto cid = makeShared<DtlsNewConnectionId>();
        cid->setConnectionIdsArraySize(length);
        for (size_t i = 0; i < length; ++i)
            cid->setConnectionIds(i, s.readUint8());
        cid->setUsage(s.readUint8());
        return cid;
    }
    if (typeInfo == typeid(DtlsRequestConnectionId)) {
        requireBytes(s, 1);
        auto request = makeShared<DtlsRequestConnectionId>();
        request->setNumCids(s.readUint8());
        return request;
    }
    if (typeInfo == typeid(DtlsHeartbeat)) {
        auto size = s.getRemainingLength().get<B>();
        requireBytes(s, 3);
        auto type = s.readUint8();
        auto length = s.readUint16Be();
        if (size > 16384 || length + 19 > size)
            throw cRuntimeError("Invalid DTLS heartbeat payload length or padding");
        auto heartbeat = makeShared<DtlsHeartbeat>();
        heartbeat->setType(type);
        heartbeat->setPayloadArraySize(length);
        for (size_t i = 0; i < length; ++i)
            heartbeat->setPayload(i, s.readUint8());
        heartbeat->setPaddingArraySize(size - 3 - length);
        for (size_t i = 0; i < heartbeat->getPaddingArraySize(); ++i)
            heartbeat->setPadding(i, s.readUint8());
        return heartbeat;
    }
    if (typeInfo == typeid(DtlsAlert)) {
        requireBytes(s, 2);
        auto alert = makeShared<DtlsAlert>();
        alert->setLevel(s.readUint8());
        alert->setDescription(s.readUint8());
        return alert;
    }
    throw cRuntimeError("Unknown DTLS chunk type");
}

} // namespace inet::dtls
