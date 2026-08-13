
//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/mrp/CfmContinuityCheckMessageSerializer.h"

#include <cstring>
#include <vector>

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/mrp/CfmContinuityCheckMessage_m.h"

namespace inet {

Register_Serializer(CfmContinuityCheckMessage, CfmContinuityCheckMessageSerializer);

namespace {

const B MAID_LENGTH = B(48);
const uint8_t FIRST_TLV_OFFSET = 70;  // the octets after this one that the TLVs follow
const uint8_t NAME_FORMAT_ABSENT = 1;
const uint8_t CFM_END_TLV = 0;      // the type of the TLV that ends the list // "the name this format belongs to is not present"

void writeName(MemoryOutputStream& stream, const char *name, const char *what)
{
    size_t length = strlen(name);
    if (length > 255)
        throw cRuntimeError("Cannot serialize CFM CCM: %s is longer than a length octet can say", what);
    stream.writeUint8(length);
    stream.writeBytes(reinterpret_cast<const uint8_t *>(name), B(length));
}

std::string readName(MemoryInputStream& stream, b maidStart, const Ptr<CfmContinuityCheckMessage>& ccm)
{
    size_t length = stream.readUint8();
    // a length that reaches past the MAID means the field is not laid out the way this
    // model reads it; take what is there and let the caller mark the chunk
    B remaining = MAID_LENGTH - B(stream.getPosition() - maidStart);
    if (B(length) > remaining)
        length = remaining.get<B>() > 0 ? remaining.get<B>() : 0;
    std::vector<uint8_t> buffer(length + 1, 0);
    if (length > 0)
        stream.readBytes(buffer.data(), B(length));
    return std::string(reinterpret_cast<const char *>(buffer.data()), length);
}

} // namespace

void CfmContinuityCheckMessageSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    // ITU Y.1731 Section 9.2.2
    const auto& ccm = staticPtrCast<const CfmContinuityCheckMessage>(chunk);
    stream.writeUint8((ccm->getMdLevel() << 5) | ccm->getVersion());
    stream.writeUint8(ccm->getOpCode());
    stream.writeUint8(ccm->getFlags());
    stream.writeUint8(FIRST_TLV_OFFSET);
    stream.writeUint32Be(ccm->getSequenceNumber());
    stream.writeUint16Be(ccm->getEndpointIdentifier());

    // the MAID: a format octet for each name, the name itself where the format says there
    // is one, and zeroes for the rest of the 48 octets
    auto maidStart = stream.getLength();
    stream.writeUint8(ccm->getMdNameFormat());
    if (ccm->getMdNameFormat() != NAME_FORMAT_ABSENT)
        writeName(stream, ccm->getMdName(), "mdName");
    stream.writeUint8(ccm->getMaNameFormat());
    if (ccm->getMaNameFormat() != NAME_FORMAT_ABSENT)
        writeName(stream, ccm->getMaName(), "maName");
    B maidLength = B(stream.getLength() - maidStart);
    if (maidLength > MAID_LENGTH)
        throw cRuntimeError("Cannot serialize CFM CCM: the names take %d B, more than the %d B of the MAID",
                (int)maidLength.get<B>(), (int)MAID_LENGTH.get<B>());
    stream.writeByteRepeatedly(0, (MAID_LENGTH - maidLength).get<B>());

    stream.writeUint32Be(0); // TxFCf
    stream.writeUint32Be(0); // RxFCb
    stream.writeUint32Be(0); // TxFCb
    stream.writeUint32Be(0); // Reserved
}

const Ptr<Chunk> CfmContinuityCheckMessageSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ccm = makeShared<CfmContinuityCheckMessage>();
    uint8_t levelAndVersion = stream.readUint8();
    ccm->setMdLevel(levelAndVersion >> 5);
    ccm->setVersion(levelAndVersion & 0x1F);
    ccm->setOpCode(stream.readUint8());
    ccm->setFlags(stream.readUint8());
    stream.readUint8(); // First TLV offset, ignored
    ccm->setSequenceNumber(stream.readUint32Be());
    ccm->setEndpointIdentifier(stream.readUint16Be());

    auto maidStart = stream.getPosition();
    ccm->setMdNameFormat(stream.readUint8());
    if (ccm->getMdNameFormat() != NAME_FORMAT_ABSENT)
        ccm->setMdName(readName(stream, maidStart, ccm).c_str());
    ccm->setMaNameFormat(stream.readUint8());
    if (ccm->getMaNameFormat() != NAME_FORMAT_ABSENT)
        ccm->setMaName(readName(stream, maidStart, ccm).c_str());
    B read = B(stream.getPosition() - maidStart);
    if (read > MAID_LENGTH) {
        // the names ran past the field they sit in, so this is not a MAID this model can
        // hold; say so rather than reporting whatever was assembled from it
        ccm->markImproperlyRepresented();
    }
    else
        stream.readByteRepeatedly(0, (MAID_LENGTH - read).get<B>());

    stream.readUint32Be(); // TxFCf, ignored
    stream.readUint32Be(); // RxFCb, ignored
    stream.readUint32Be(); // TxFCb, ignored
    stream.readUint32Be(); // Reserved, ignored
    return ccm;
}

Register_Serializer(CfmTlvBase, CfmTlvSerializer);
Register_Serializer(CfmEndTlv, CfmTlvSerializer);
Register_Serializer(CfmTlvRaw, CfmTlvSerializer);

void CfmTlvSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& tlv = staticPtrCast<const CfmTlvBase>(chunk);
    stream.writeUint8(tlv->getType());
    if (tlv->getType() == CFM_END_TLV)
        return;
    const auto& rawTlv = dynamicPtrCast<const CfmTlvRaw>(chunk);
    if (rawTlv == nullptr)
        throw cRuntimeError("Cannot serialize '%s': its type is %d, which is not the TLV that ends the list, and only a raw TLV holds what such a one carries",
                chunk->getClassName(), tlv->getType());
    if (rawTlv->getLength() != rawTlv->getBytesArraySize())
        throw cRuntimeError("Cannot serialize CFM TLV of type %d: it declares %d B and holds %d",
                rawTlv->getType(), rawTlv->getLength(), (int)rawTlv->getBytesArraySize());
    stream.writeUint16Be(rawTlv->getLength());
    for (size_t i = 0; i < rawTlv->getBytesArraySize(); i++)
        stream.writeByte(rawTlv->getBytes(i));
}

const Ptr<Chunk> CfmTlvSerializer::deserialize(MemoryInputStream& stream) const
{
    uint8_t type = stream.readUint8();
    if (type == CFM_END_TLV)
        return makeShared<CfmEndTlv>();
    // what is inside the other TLVs is not modelled, but the type, the length and the
    // octets they carry are enough to write them out again as they were
    auto tlv = makeShared<CfmTlvRaw>();
    tlv->setType(type);
    tlv->setLength(stream.readUint16Be());
    tlv->setBytesArraySize(tlv->getLength());
    for (size_t i = 0; i < tlv->getLength(); i++)
        tlv->setBytes(i, stream.readByte());
    return tlv;
}

} // namespace inet
