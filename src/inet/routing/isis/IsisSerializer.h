//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Byte serializers for the IS-IS PDUs, so they can be put on the wire and so
// the ~tND (packet-data) fingerprint can be computed. The on-wire layout uses
// the ISO/IEC 10589 common 8-byte header followed by the PDU's fixed fields and
// its variable parts, each prefixed by a one-byte element count.
//

#ifndef __INET_ISISSERIALIZER_H
#define __INET_ISISSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {
namespace isis {

class INET_API IsisLanHelloSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    IsisLanHelloSerializer() : FieldsChunkSerializer() {}
};

class INET_API IsisPtpHelloSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    IsisPtpHelloSerializer() : FieldsChunkSerializer() {}
};

class INET_API IsisLspSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    IsisLspSerializer() : FieldsChunkSerializer() {}
};

class INET_API IsisCsnpSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    IsisCsnpSerializer() : FieldsChunkSerializer() {}
};

class INET_API IsisPsnpSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    IsisPsnpSerializer() : FieldsChunkSerializer() {}
};

} // namespace isis
} // namespace inet

#endif
