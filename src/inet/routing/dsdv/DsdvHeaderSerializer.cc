// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/routing/dsdv/DsdvHeaderSerializer.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/routing/dsdv/DsdvTable.h"

namespace inet {
namespace dsdv {
Register_Serializer(DsdvHeader, DsdvHeaderSerializer);

void DsdvHeaderSerializer::serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const DsdvHeader>(chunk);
    // Wire encoding permits INT_MAX; reception rejects it before adding a hop.
    int entryBytes = DsdvTable::getEntryBytes(header->getVersion());
    if (entryBytes == 0 || header->getEntriesArraySize() > UINT16_MAX || header->getChunkLength() !=
            B(DsdvTable::HEADER_BYTES + entryBytes * header->getEntriesArraySize()))
        throw cRuntimeError("DSDV header count/length mismatch");
    for (size_t i = 0; i < header->getEntriesArraySize(); i++)
        if (header->getVersion() == 1 && header->getEntries(i).defaultCapability)
            throw cRuntimeError("DSDV version 1 cannot encode default capability");
    stream.writeUint8(header->getVersion());
    stream.writeUint8(header->getKind());
    stream.writeUint16Be(header->getEntriesArraySize());
    stream.writeIpv4Address(header->getSender());
    stream.writeUint64Be(header->getSelfSequence());
    for (size_t i = 0; i < header->getEntriesArraySize(); i++) {
        const auto& entry = header->getEntries(i);
        stream.writeIpv4Address(entry.destination);
        stream.writeUint64Be(entry.sequence);
        stream.writeUint32Be(entry.metric);
        if (header->getVersion() == 2)
            stream.writeUint8(entry.defaultCapability ? 1 : 0);
    }
}

const Ptr<Chunk> DsdvHeaderSerializer::deserializeFields(MemoryInputStream& stream, const std::type_info&) const
{
    auto header = makeShared<DsdvHeader>();
    auto length = stream.getRemainingLength();
    if (length < B(DsdvTable::HEADER_BYTES)) {
        header->markIncorrect();
        stream.readByteRepeatedly(0, length.get<B>());
        return header;
    }
    header->setVersion(stream.readUint8());
    header->setKind(stream.readUint8());
    size_t count = stream.readUint16Be();
    header->setSender(stream.readIpv4Address());
    header->setSelfSequence(stream.readUint64Be());
    int entryBytes = DsdvTable::getEntryBytes(header->getVersion());
    if (entryBytes == 0 || length != B(DsdvTable::HEADER_BYTES + entryBytes * count)) {
        header->markIncorrect();
        stream.readByteRepeatedly(0, stream.getRemainingLength().get<B>());
        return header;
    }
    header->setEntriesArraySize(count);
    for (size_t i = 0; i < count; i++) {
        DsdvEntry entry;
        entry.destination = stream.readIpv4Address();
        entry.sequence = stream.readUint64Be();
        entry.metric = stream.readUint32Be();
        if (header->getVersion() == 2) {
            uint8_t flags = stream.readUint8();
            entry.defaultCapability = flags & 1;
            if (flags & ~uint8_t(1))
                header->markIncorrect();
        }
        header->setEntries(i, entry);
    }
    header->setChunkLength(length);
    if (!DsdvTable::isValidHeader(*header))
        header->markIncorrect();
    return header;
}
} // namespace dsdv
} // namespace inet
