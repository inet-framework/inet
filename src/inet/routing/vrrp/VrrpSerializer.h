//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// VRRPv2 (RFC 3768) ported from the ANSAINET project.
// Original authors: Petr Vitek, Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_VRRPSERIALIZER_H
#define __INET_VRRPSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {
namespace vrrp {

/**
 * Converts between VrrpAdvertisement and binary (network byte order) data.
 */
class INET_API VrrpSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    VrrpSerializer() : FieldsChunkSerializer() {}
};

} // namespace vrrp
} // namespace inet

#endif
