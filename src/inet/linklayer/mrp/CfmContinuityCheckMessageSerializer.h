//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_CFMCONTINUITYHECKMESSAGESERIALIZER_H
#define __INET_CFMCONTINUITYHECKMESSAGESERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Serializer for CFM CCM messages.
 */
class INET_API CfmContinuityCheckMessageSerializer : public FieldsChunkSerializer
{
public:
    using FieldsChunkSerializer::FieldsChunkSerializer;
    virtual void serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserializeFields(MemoryInputStream& stream, const std::type_info&) const override;
};

/**
 * Converts between CfmTlvBase and the binary (network byte order) TLVs of a CFM PDU.
 * Deserializing yields the TLV the type octet names.
 */
class INET_API CfmTlvSerializer : public FieldsChunkSerializer
{
public:
    using FieldsChunkSerializer::FieldsChunkSerializer;
    virtual void serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserializeFields(MemoryInputStream& stream, const std::type_info&) const override;
};

} // namespace inet

#endif
