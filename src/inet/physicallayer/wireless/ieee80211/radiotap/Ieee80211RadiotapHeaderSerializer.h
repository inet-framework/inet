//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211RADIOTAPHEADERSERIALIZER_H
#define __INET_IEEE80211RADIOTAPHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

namespace physicallayer {

/**
 * Returns the length of a Radiotap header that has the fields of the given
 * present word, with the alignment that the format specifies. The present
 * word must contain only the fields of Ieee80211RadiotapHeader.
 */
INET_API B computeIeee80211RadiotapHeaderLength(uint32_t present);

/**
 * Converts between Ieee80211RadiotapHeader and the little-endian Radiotap
 * header of a capture record. The deserializer stops at the first field that
 * it cannot represent, and it marks the chunk as improperly represented. The
 * chunk then keeps the length of the header in the record.
 */
class INET_API Ieee80211RadiotapHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserializeFields(MemoryInputStream& stream, const std::type_info&) const override;

  public:
    Ieee80211RadiotapHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace physicallayer

} // namespace inet

#endif
