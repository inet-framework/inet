//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FRAGMENTNUMBERHEADERSERIALIZER_H
#define __INET_FRAGMENTNUMBERHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between FragmentNumberHeader and binary (network byte order) fragment number header.
 */
class INET_API FragmentNumberHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    FragmentNumberHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

