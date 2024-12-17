//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SHORTCUTMACHEADERSERIALIZER_H
#define __INET_SHORTCUTMACHEADERSERIALIZER_H

#include "inet/common/packet/serializer/ChunkSerializer.h"

namespace inet {
namespace physicallayer {

class INET_API ShortcutMacHeaderSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const;
};

} // namespace physicallayer
} // namespace inet

#endif

