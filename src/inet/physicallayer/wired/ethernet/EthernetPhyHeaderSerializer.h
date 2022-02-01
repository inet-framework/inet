//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETPHYHEADERSERIALIZER_H
#define __INET_ETHERNETPHYHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

namespace physicallayer {

/**
 * Converts between EthernetPhyHeaderBase and binary (network byte order) Ethernet PHY header.
 */
class INET_API EthernetPhyHeaderBaseSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    EthernetPhyHeaderBaseSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between EthernetPhyHeader and binary (network byte order) Ethernet PHY header.
 */
class INET_API EthernetPhyHeaderSerializer : public FieldsChunkSerializer
{
    friend EthernetPhyHeaderBaseSerializer;

  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    EthernetPhyHeaderSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between EtherFragmentPhyHeader and binary (network byte order) Ethernet fragment PHY header.
 */
class INET_API EthernetFragmentPhyHeaderSerializer : public FieldsChunkSerializer
{
    friend EthernetPhyHeaderBaseSerializer;

  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    EthernetFragmentPhyHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace physicallayer

} // namespace inet

#endif

