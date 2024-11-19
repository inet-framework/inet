//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/mrp/CfmContinuityCheckMessageSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/mrp/CfmContinuityCheckMessage_m.h"

namespace inet {

Register_Serializer(CfmContinuityCheckMessage, CfmContinuityCheckMessageSerializer);

void CfmContinuityCheckMessageSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    // ITU Y.1731 Section 9.2.2
    const auto& ccm = staticPtrCast<const CfmContinuityCheckMessage>(chunk);
    stream.writeUint8(ccm->getMdLevel());  // MD Level + Version
    stream.writeUint8(ccm->getOpCode());
    stream.writeUint8(ccm->getFlags());
    stream.writeUint8(70); // First TLV offset
    stream.writeUint32Be(ccm->getSequenceNumber());
    stream.writeUint16Be(ccm->getEndpointIdentifier());

    // MEG ID
    stream.writeUint8(0); // reserved
    stream.writeUint8(4); // format
    size_t nameLength = strlen(ccm->getMessageName());
    ASSERT(nameLength <= 45);
    stream.writeUint8(nameLength);
    stream.writeBytes(reinterpret_cast<const uint8_t*>(ccm->getMessageName()), B(nameLength));
    stream.writeByteRepeatedly(0, 45 - nameLength);

    stream.writeUint32Be(0); // TxFCf
    stream.writeUint32Be(0); // RxFCb
    stream.writeUint32Be(0); // TxFCb
    stream.writeUint32Be(0); // Reserved
    stream.writeUint8(0);  // End TLV
}

const Ptr<Chunk> CfmContinuityCheckMessageSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ccm = makeShared<CfmContinuityCheckMessage>();
    ccm->setMdLevel(stream.readUint8());
    ccm->setOpCode(stream.readUint8());
    ccm->setFlags(stream.readUint8());
    stream.readUint8(); // First TLV offset, ignored
    ccm->setSequenceNumber(stream.readUint32Be());
    ccm->setEndpointIdentifier(stream.readUint16Be());

    stream.readUint8(); // reserved, ignored
    stream.readUint8(); // format, ignored
    int nameLength = stream.readUint8();
    ASSERT(nameLength <= 45);
    uint8_t buffer[46];
    stream.readBytes(buffer, B(nameLength));
    buffer[nameLength] = '\0';
    ccm->setMessageName(reinterpret_cast<const char*>(buffer));

    stream.readUint32Be(); // TxFCf, ignored
    stream.readUint32Be(); // RxFCb, ignored
    stream.readUint32Be(); // TxFCb, ignored
    stream.readUint32Be(); // Reserved, ignored
    stream.readUint8(); // End TLV, ignored
    return ccm;
}

} // namespace inet

