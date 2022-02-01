//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CSMACAMACHEADERSERIALIZER_H
#define __INET_CSMACAMACHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/linklayer/csmaca/CsmaCaMacHeader_m.h"

namespace inet {

/**
 * Converts between CsmaCaMacHeader and binary network byte order mac header.
 */
class INET_API CsmaCaMacHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    CsmaCaMacHeaderSerializer() : FieldsChunkSerializer() {}
};

class INET_API CsmaCaMacTrailerSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    CsmaCaMacTrailerSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

