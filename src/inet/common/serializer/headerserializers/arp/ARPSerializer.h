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

#ifndef ARPSERIALIZER_H_
#define ARPSERIALIZER_H_

#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "inet/common/serializer/headers/defs.h"

namespace inet {

namespace serializer {
/**
 * Converts between ARPPacket and binary (network byte order)  ARP header.
 */

class ARPSerializer
{
    public:

        /**
         * Serializes an ARPPacket for transmission on the wire.
         * Returns the length of data written into buffer.
         */
        int serialize(const ARPPacket *pkt, unsigned char *buf, unsigned int bufsize);

        /**
         * Puts a packet sniffed from the wire into an ARPPacket.
         */
        void parse(const unsigned char *buf, unsigned int bufsize, ARPPacket *pkt);
};

} // namespace serializer
} // namespace inet

#endif /* ARPSERIALIZER_H_ */
