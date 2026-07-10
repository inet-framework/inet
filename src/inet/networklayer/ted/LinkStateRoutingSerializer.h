//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_LINKSTATEROUTINGSERIALIZER_H
#define __INET_LINKSTATEROUTINGSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/networklayer/ted/LinkStatePacket_m.h"

namespace inet {

/**
 * LinkStateSerializer format v1 -- NOT an RFC wire format. LinkStateMsg (see
 * LinkStatePacket.msg) is this model's own internal flooding protocol used by
 * LinkStateRouting to mirror the local Traffic Engineering Database (TED, see
 * Ted.msg) to RSVP-TE peers; no standard defines its bytes. This class fixes
 * a single, deterministic, documented byte layout for it so that packets
 * carrying it can be serialized (required for the `~tND` fingerprint
 * ingredient, which needs every in-flight chunk type to have a serializer).
 *
 * Layout (all multi-byte integers network byte order / big-endian):
 *
 *   Message header (4 bytes):
 *     uint8_t  command                 (LinkStateMsg::command, not RFC-anything)
 *     uint8_t  flags                   (bit0 = request; bits 1-7 reserved, 0)
 *     uint16_t count                   (number of TeLinkStateInfo records that follow)
 *
 *   Then `count` fixed-width TeLinkStateInfo records (116 bytes each, see
 *   LinkStateRoutingSerializer.cc for the exact per-field layout and the
 *   rationale for each field's width/handling).
 *
 * See LinkStateRoutingSerializer.cc for the full per-record field table and
 * for why LinkStateRouting.cc's chunk-length formula had to change to match.
 */
class INET_API LinkStateRoutingSerializer : public FieldsChunkSerializer
{
  private:
    static void serializeLinkStateInfo(MemoryOutputStream& stream, const TeLinkStateInfo& info);
    static TeLinkStateInfo deserializeLinkStateInfo(MemoryInputStream& stream);

  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    LinkStateRoutingSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif
