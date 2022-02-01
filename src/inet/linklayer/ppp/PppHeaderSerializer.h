//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PPPHEADERSERIALIZER_H
#define __INET_PPPHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between PppHeader and binary (network byte order) Ppp header.
 */
class INET_API PppHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    PppHeaderSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between PppTrailer and binary (network byte order) Ppp trailer.
 */
class INET_API PppTrailerSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    PppTrailerSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

