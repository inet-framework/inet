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

#ifndef __INET_ARPSERIALIZER_H
#define __INET_ARPSERIALIZER_H

#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "inet/common/serializer/SerializerBase.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"

namespace inet {

namespace serializer {

/**
 * Converts between ARPPacket and binary (network byte order)  ARP header.
 */
class INET_API ARPSerializer : public SerializerBase
{
  protected:
    /**
     * Serializes an ARPPacket for transmission on the wire.
     * Returns the length of data written into buffer.
     */
    virtual void serialize(const cPacket *pkt, Buffer &b, Context& context) override;

    /**
     * Puts a packet sniffed from the wire into an ARPPacket.
     */
    virtual cPacket *deserialize(const Buffer &b, Context& context) override;

    MACAddress readMACAddress(const Buffer& b, unsigned int size);
    IPv4Address readIPv4Address(const Buffer& b, unsigned int size);

  public:
    ARPSerializer(const char *name = nullptr) : SerializerBase(name) {}

    //TODO remove next 2 functions
        /**
         * Serializes an ARPPacket for transmission on the wire.
         * Returns the length of data written into buffer.
         */
        int serialize(const ARPPacket *pkt, unsigned char *buf, unsigned int bufsize)
        { Buffer b(buf, bufsize); Context c; serialize(pkt, b, c); return b.getPos(); }

        /**
         * Puts a packet sniffed from the wire into an ARPPacket.
         */
        ARPPacket *parse(const unsigned char *buf, unsigned int bufsize)
        { Buffer b(const_cast<unsigned char *>(buf), bufsize); Context c; return check_and_cast<ARPPacket *>(deserialize(b, c)); }
};

} // namespace serializer
} // namespace inet

#endif /* ARPSERIALIZER_H_ */
