//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE802EPDHEADERSERIALIZER_H
#define __INET_IEEE802EPDHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

class INET_API Ieee802EpdHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee802EpdHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

