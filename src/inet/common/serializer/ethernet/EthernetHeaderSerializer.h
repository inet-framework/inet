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

#include "inet/common/packet/Serializer.h"

namespace inet {

namespace serializer {

/**
 * Converts between EtherMacHeader and binary (network byte order) Ethernet mac header.
 */
class INET_API EthernetMacHeaderSerializer : public FieldsChunkSerializer
{
  public:
    EthernetMacHeaderSerializer() : FieldsChunkSerializer() {}

    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const;
};

class INET_API EthernetPaddingSerializer : public FieldsChunkSerializer
{
  public:
    EthernetPaddingSerializer() : FieldsChunkSerializer() {}

    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const;
};

class INET_API EthernetFcsSerializer : public FieldsChunkSerializer
{
  public:
    EthernetFcsSerializer() : FieldsChunkSerializer() {}

    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const;
};

/**
 * Converts between EtherPhyHeader and binary (network byte order) Ethernet phy header.
 */
class INET_API EthernetPhyHeaderSerializer : public FieldsChunkSerializer
{
  public:
    EthernetPhyHeaderSerializer() : FieldsChunkSerializer() {}

    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const;
};

} // namespace serializer

} // namespace inet

#endif // ifndef __INET_ETHERNETHEADERSERIALIZER_H
