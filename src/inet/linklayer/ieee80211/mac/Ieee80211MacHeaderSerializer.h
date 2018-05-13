//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
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
    virtual void parseDataOrMgmtFrame(MemoryInputStream &stream, const Ptr<ieee80211::Ieee80211DataOrMgmtHeader> frame, inet::ieee80211::Ieee80211FrameType type, uint8_t fc1) const;

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

} // namespace ieee80211

} // namespace inet

#endif // ifndef __INET_IEEE80211MACHEADERSERIALIZER_H
