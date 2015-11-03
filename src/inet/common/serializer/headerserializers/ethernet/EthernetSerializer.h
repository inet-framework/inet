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

#ifndef __INET_ETHERNETSERIALIZER_H
#define __INET_ETHERNETSERIALIZER_H

#include "inet/common/serializer/headers/defs.h"
#include "inet/common/serializer/SerializerBase.h"
#include "inet/linklayer/ethernet/EtherFrame.h"

namespace inet {

namespace serializer {

/**
 * Converts between EtherFrame and binary (network byte order) Ethernet header.
 */
class INET_API EthernetSerializer : public SerializerBase
{
  protected:
    /**
     * Serializes an EtherFrame for transmission on the wire.
     */
    virtual void serialize(const cPacket *pkt, Buffer &b, Context& context) override;

    /**
     * Puts a packet sniffed from the wire into an EtherFrame.
     */
    virtual cPacket *deserialize(const Buffer &b, Context& context) override;

  public:
    EthernetSerializer(const char *name = nullptr) : SerializerBase(name) {}
};

} // namespace serializer

} // namespace inet

#endif /* ETHERNETSERIALIZER_H_ */
