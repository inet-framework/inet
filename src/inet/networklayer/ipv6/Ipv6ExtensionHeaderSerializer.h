//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV6EXTENSIONHEADERSERIALIZER_H
#define __INET_IPV6EXTENSIONHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

class INET_API Ipv6HopByHopOptionsHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
  public:
    Ipv6HopByHopOptionsHeaderSerializer() : FieldsChunkSerializer() {}
};

class INET_API Ipv6DestinationOptionsHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
  public:
    Ipv6DestinationOptionsHeaderSerializer() : FieldsChunkSerializer() {}
};

class INET_API Ipv6RoutingHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
  public:
    Ipv6RoutingHeaderSerializer() : FieldsChunkSerializer() {}
};

class INET_API Ipv6FragmentHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
  public:
    Ipv6FragmentHeaderSerializer() : FieldsChunkSerializer() {}
};

class INET_API Ipv6AuthenticationHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
  public:
    Ipv6AuthenticationHeaderSerializer() : FieldsChunkSerializer() {}
};

class INET_API Ipv6EncapsulatingSecurityPayloadHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
  public:
    Ipv6EncapsulatingSecurityPayloadHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif
