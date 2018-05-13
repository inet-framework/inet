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

#ifndef __INET_TCPHEADERSERIALIZER_H
#define __INET_TCPHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {

namespace tcp {

/**
 * Converts between TcpHeader and binary (network byte order) Tcp header.
 */
class INET_API TcpHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serializeOption(MemoryOutputStream& stream, const TcpOption *option) const;
    virtual TcpOption *deserializeOption(MemoryInputStream& stream) const;

    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    TcpHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCPHEADERSERIALIZER_H
