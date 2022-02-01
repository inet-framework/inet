//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SEQUENCENUMBERHEADERSERIALIZER_H
#define __INET_SEQUENCENUMBERHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between SequenceNumberHeader and binary (network byte order) sequence number header.
 */
class INET_API SequenceNumberHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    SequenceNumberHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

