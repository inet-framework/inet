//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211MACHEADERSERIALIZER_H
#define __INET_IEEE80211MACHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {

namespace ieee80211 {

/**
 * Converts between Ieee80211MacHeader and binary network byte order IEEE 802.11 mac header.
 */
class INET_API Ieee80211MacHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211MacHeaderSerializer() : FieldsChunkSerializer() {}
};

class INET_API Ieee80211MacTrailerSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211MacTrailerSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between Ieee80211MsduSubframeHeader and binary network byte order IEEE 802.11 Msdu header.
 */
class INET_API Ieee80211MsduSubframeHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211MsduSubframeHeaderSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between Ieee80211MpduSubframeHeader and binary network byte order IEEE 802.11 Mpdu header.
 */
class INET_API Ieee80211MpduSubframeHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211MpduSubframeHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace ieee80211

} // namespace inet

#endif

