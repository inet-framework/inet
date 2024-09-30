//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MRPPDUSERIALIZER_H
#define __INET_MRPPDUSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between PDU and binary (network byte order) MRP PDU packets.
 */
class INET_API MrpTlvSerializer: public FieldsChunkSerializer {
protected:
    virtual void serialize(MemoryOutputStream &stream, const Ptr<const Chunk> &chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream &stream) const override;

public:
    MrpTlvSerializer() :
            FieldsChunkSerializer() {
    }
};

class INET_API MrpVersionFieldSerializer: public FieldsChunkSerializer {
protected:
    virtual void serialize(MemoryOutputStream &stream, const Ptr<const Chunk> &chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream &stream) const override;

public:
    MrpVersionFieldSerializer() :
            FieldsChunkSerializer() {
    }
};
class INET_API MrpSubTlvSerializer: public FieldsChunkSerializer {
protected:
    virtual void serialize(MemoryOutputStream &stream, const Ptr<const Chunk> &chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream &stream) const override;

public:
    MrpSubTlvSerializer() :
            FieldsChunkSerializer() {
    }
};

} // namespace inet

#endif

