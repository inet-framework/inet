//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211MGMTFRAMESERIALIZER_H
#define __INET_IEEE80211MGMTFRAMESERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {

namespace ieee80211 {

/**
 * Converts between Ieee80211MgmtFrame and binary network byte order IEEE 802.11 mgmt frame.
 */
class INET_API Ieee80211MgmtFrameSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    // the concrete management-frame subtype cannot be told apart from the body bytes
    // alone (the subtype lives in the already-consumed MAC header), so dispatch on the
    // requested type_info; the plain deserialize(stream) overload is therefore unused
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const override;

    // set by deserialize(stream, typeInfo) just before it delegates to the base class,
    // and read back by deserialize(stream) to pick the subtype (single-threaded use)
    mutable const std::type_info *requestedTypeInfo = nullptr;

  public:
    Ieee80211MgmtFrameSerializer() : FieldsChunkSerializer() {}
};

} // namespace ieee80211

} // namespace inet

#endif

