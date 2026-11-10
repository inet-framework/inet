// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_IEEE80211MESHFRAMESERIALIZER_H
#define __INET_IEEE80211MESHFRAMESERIALIZER_H

#include <functional>

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MeshFrame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API Ieee80211MeshFrameSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserializeFields(MemoryInputStream& stream, const std::type_info& typeInfo) const override;

  public:
    // Shared with the existing beacon/probe codec. The caller has checked the IE boundary.
    static bool isMeshElement(uint8_t id);
    static bool readElement(MemoryInputStream& stream, uint8_t id, uint8_t length, Ieee80211MeshFrame& frame, int peeringAction = 0);
    static void writeElements(MemoryOutputStream& stream, const Ieee80211MeshFrame& frame, const std::function<void()>& beforeElement = {}, bool includeHt = true);
};

class INET_API Ieee80211MeshControlSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserializeFields(MemoryInputStream& stream, const std::type_info& typeInfo) const override;
};

} // namespace ieee80211
} // namespace inet

#endif
