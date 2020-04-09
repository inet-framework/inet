//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __INET_ETHERNETHEADERSERIALIZER_H
#define __INET_ETHERNETHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between EtherMacHeader and binary (network byte order) Ethernet mac header.
 */
class INET_API EthernetMacHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    EthernetMacHeaderSerializer() : FieldsChunkSerializer() {}
};

class INET_API EthernetControlFrameSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    EthernetControlFrameSerializer() : FieldsChunkSerializer() {}
};

class INET_API EthernetPaddingSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    EthernetPaddingSerializer() : FieldsChunkSerializer() {}
};

class INET_API EthernetFcsSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    EthernetFcsSerializer() : FieldsChunkSerializer() {}
};

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

} // namespace inet

#endif // ifndef __INET_ETHERNETHEADERSERIALIZER_H
