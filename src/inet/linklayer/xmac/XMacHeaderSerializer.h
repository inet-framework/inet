//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_XMACHEADERSERIALIZER_H
#define __INET_XMACHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between XMacHeader and binary (network byte order) XMAC headers.
 */
class INET_API XMacHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    XMacHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

