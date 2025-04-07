//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CHECKSUMHEADERSERIALIZER_H
#define __INET_CHECKSUMHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between ChecksumHeader and binary (network byte order) CRC header.
 */
class INET_API ChecksumHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    ChecksumHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

