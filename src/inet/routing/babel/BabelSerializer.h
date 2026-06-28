//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_BABELSERIALIZER_H
#define __INET_BABELSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/routing/babel/BabelMessage_m.h"

namespace inet {
namespace babel {

/**
 * Serializes/deserializes the 4-byte Babel message header (RFC 6126, 4.2).
 */
class INET_API BabelHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    BabelHeaderSerializer() : FieldsChunkSerializer() {}
};

/**
 * Serializes/deserializes any Babel TLV (RFC 6126, 4.5). The concrete TLV is
 * selected by the leading type octet; one serializer handles the whole TLV
 * hierarchy.
 */
class INET_API BabelTlvSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    BabelTlvSerializer() : FieldsChunkSerializer() {}
};

/**
 * Wire length of a TLV, so callers can set its chunkLength before it is made
 * immutable (FieldsChunkSerializer asserts serialized bytes == chunkLength).
 */
B babelTlvLength(const BabelTlv *tlv);

} // namespace babel
} // namespace inet

#endif
