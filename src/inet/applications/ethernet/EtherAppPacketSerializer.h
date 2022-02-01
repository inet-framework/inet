//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERAPPPACKETSERIALIZER_H
#define __INET_ETHERAPPPACKETSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between EtherAppReq and binary (network byte order) application packet.
 */
class INET_API EtherAppReqSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    EtherAppReqSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between EtherAppResp and binary (network byte order) application packet.
 */
class INET_API EtherAppRespSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    EtherAppRespSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

