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

#ifndef ETHERNETSERIALIZER_H_
#define ETHERNETSERIALIZER_H_

#include "EtherFrame.h"

#include "Address.h"

#include "headers/defs.h"

/**
 * Converts between EtherFrame and binary (network byte order) Ethernet header.
 */

class EthernetSerializer
{
    public:

        /**
         * Serializes an EtherFrame for transmission on the wire.
         * Returns the length of data written into buffer.
         */
        int serialize(const EthernetIIFrame *pkt, unsigned char *buf, unsigned int bufsize);

        /**
         * Puts a packet sniffed from the wire into an EtherFrame.
         */
        void parse(const unsigned char *buf, unsigned int bufsize, EthernetIIFrame *pkt);
};

#endif /* ETHERNETSERIALIZER_H_ */
