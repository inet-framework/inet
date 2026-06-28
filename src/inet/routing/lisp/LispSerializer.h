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
#include "inet/routing/lisp/LispMessages_m.h"

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

/**
 * Serializes/deserializes the LISP control messages (Map-Request/Reply/Register/Notify,
 * RFC 6830, section 6), including their AFI-encoded addresses and EID-to-RLOC records.
 * The concrete message is selected by the leading type octet.
 */
class INET_API LispControlMessageSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    LispControlMessageSerializer() : FieldsChunkSerializer() {}
};

// Wire-length helpers so the modules can set each chunk's exact length before it is serialized.
B lispAfiAddressLength(const L3Address& address);
B lispMapRecordLength(const LispMapRecord& record);
B lispMapRegisterLength(const LispMapRegister& msg);
B lispMapReplyLength(const LispMapReply& msg);
B lispMapRequestLength(const LispMapRequest& msg);

} // namespace lisp
} // namespace inet

#endif
