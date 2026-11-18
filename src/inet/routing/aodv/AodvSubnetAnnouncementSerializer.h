// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_AODVSUBNETANNOUNCEMENTSERIALIZER_H
#define __INET_AODVSUBNETANNOUNCEMENTSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {
namespace aodv {

class INET_API AodvSubnetAnnouncementSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserializeFields(MemoryInputStream& stream, const std::type_info&) const override;
};

} // namespace aodv
} // namespace inet

#endif
