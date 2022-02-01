//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETMACHEADERSERIALIZER_H
#define __INET_ETHERNETMACHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between EthernetMacAddressFields and binary (network byte order) 802.3 MAC addresses header.
 */
class INET_API EthernetMacAddressFieldsSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    EthernetMacAddressFieldsSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between EthernetTypeOrLengthField and binary (network byte order) 802.3 MAC type or length header.
 */
class INET_API EthernetTypeOrLengthFieldSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    EthernetTypeOrLengthFieldSerializer() : FieldsChunkSerializer() {}
};

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

} // namespace inet

#endif

