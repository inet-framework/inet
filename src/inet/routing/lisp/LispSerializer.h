//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_LISPSERIALIZER_H
#define __INET_LISPSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {
namespace lisp {

/**
 * Serializes/deserializes the LISP data-plane header (RFC 6830, section 5.3).
 */
class INET_API LispHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    LispHeaderSerializer() : FieldsChunkSerializer() {}
};

// TODO control-message serializers (Map-Request/Reply/Register/Notify) need the
// modules to set the exact chunkLength of their variable-length records first.

} // namespace lisp
} // namespace inet

#endif
