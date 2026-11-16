// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DTLSPACKETSERIALIZER_H
#define INET_DTLSPACKETSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet::dtls {

class INET_API DtlsPacketSerializer : public FieldsChunkSerializer
{
  protected:
    void serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    const Ptr<Chunk> deserializeFields(MemoryInputStream& stream, const std::type_info& typeInfo) const override;
};

} // namespace inet::dtls
#endif
