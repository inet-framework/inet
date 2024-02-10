#ifndef __INET_MRPPDUSERIALIZER_H
#define __INET_MRPPDUSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between PDU and binary (network byte order) MRP PDU packets.
 */
class INET_API mrpTlvSerializer : public FieldsChunkSerializer
{
protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

public:
    mrpTlvSerializer() : FieldsChunkSerializer() {}
};

class INET_API mrpVersionFieldSerializer : public FieldsChunkSerializer
{
protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

public:
    mrpVersionFieldSerializer() : FieldsChunkSerializer() {}
};
class INET_API mrpSubTlvSerializer : public FieldsChunkSerializer
{
protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

public:
    mrpSubTlvSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

