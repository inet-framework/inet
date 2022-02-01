//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __UNITTEST_H_
#define __UNITTEST_H_

#include "inet/common/packet/ChunkQueue.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/common/packet/serializer/SequenceChunkSerializer.h"
#include "UnitTest_m.h"

namespace inet {

class CompoundHeader : public CompoundHeader_Base
{
  public:
    CompoundHeader() : CompoundHeader_Base() { }
    CompoundHeader(const CompoundHeader& other) : CompoundHeader_Base(other) { }
    CompoundHeader& operator=(const CompoundHeader& other) {if (this==&other) return *this; CompoundHeader_Base::operator=(other); return *this;}
    virtual CompoundHeader *dup() const {return new CompoundHeader(*this);}
};

class ApplicationHeaderSerializer : public FieldsChunkSerializer
{
  public:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
};

class TcpHeaderSerializer : public FieldsChunkSerializer
{
  public:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
};

class IpHeaderSerializer : public FieldsChunkSerializer
{
  public:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
};

class EthernetHeaderSerializer : public FieldsChunkSerializer
{
  public:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
};

class EthernetTrailerSerializer : public FieldsChunkSerializer
{
  public:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
};

class CompoundHeaderSerializer : public SequenceChunkSerializer
{
  public:
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const override;
};

class TlvHeaderSerializer : public FieldsChunkSerializer
{
  public:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
};

class TlvHeaderBoolSerializer : public FieldsChunkSerializer
{
  public:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
};

class TlvHeaderIntSerializer : public FieldsChunkSerializer
{
  public:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
};

class UnitTest : public cSimpleModule
{
  protected:
    void initialize() override;
};

} // namespace

#endif // #ifndef __UNITTEST_H_

