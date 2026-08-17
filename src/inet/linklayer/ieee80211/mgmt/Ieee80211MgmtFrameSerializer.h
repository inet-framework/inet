//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211MGMTFRAMESERIALIZER_H
#define __INET_IEEE80211MGMTFRAMESERIALIZER_H

#include <typeinfo>

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {

namespace ieee80211 {

/**
 * Converts between Ieee80211MgmtFrame and binary network byte order IEEE 802.11 mgmt frame.
 * The input stream passed to deserialize() must be bounded to the exact management-frame body;
 * all bytes remaining after the fixed fields are interpreted as management elements.
 */
class INET_API Ieee80211MgmtFrameSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    static const Ptr<Chunk> deserializeFrame(MemoryInputStream& stream, const std::type_info& typeInfo);

  public:
    Ieee80211MgmtFrameSerializer() : FieldsChunkSerializer() {}
};

template<typename Frame>
class Ieee80211TypedMgmtFrameSerializer : public Ieee80211MgmtFrameSerializer
{
  protected:
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override
    {
        return deserializeFrame(stream, typeid(Frame));
    }
};

} // namespace ieee80211

} // namespace inet

#endif
