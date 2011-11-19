//
// Copyright (C) 2004 Andras Varga
//               2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_UDPSERIALIZER_H
#define __INET_UDPSERIALIZER_H

#include "UDPPacket.h"


/**
 * Converts between UDPPacket and binary (network byte order) UDP header.
 */
class UDPSerializer
{
    public:
        UDPSerializer() {}

        /**
         * Serializes an UDPPacket for transmission on the wire.
         * Returns the length of data written into buffer.
         */
        int serialize(const UDPPacket *pkt, unsigned char *buf, unsigned int bufsize);

        /**
         * Puts a packet sniffed from the wire into an UDPPacket.
         */
        void parse(const unsigned char *buf, unsigned int bufsize, UDPPacket *pkt);
};

#endif

