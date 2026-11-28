// SPDX-License-Identifier: LGPL-3.0-or-later
//
#ifndef __INET_IEEE802154FRAMESERIALIZER_H
#define __INET_IEEE802154FRAMESERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
namespace inet {
class INET_API Ieee802154FrameSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserializeFields(MemoryInputStream& stream, const std::type_info&) const override;
};
}
#endif
