//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen
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

#ifndef __INET_IPSERIALIZER_H
#define __INET_IPSERIALIZER_H

#include "IPDatagram.h"


/**
 * Converts between IPDatagram and binary (network byte order) IP header.
 */
class IPSerializer
{
    public:
        IPSerializer() {}

        /**
         * Serializes an IPDatagram for transmission on the wire.
         * The checksum is NOT filled in. (The kernel does that when sending
         * the frame over a raw socket.)
         * Returns the length of data written into buffer.
         */
        int serialize(const IPDatagram *dgram, unsigned char *buf, unsigned int bufsize);

        /**
         * Puts a packet sniffed from the wire into an IPDatagram. Does NOT
         * verify the checksum.
         */
        void parse(const unsigned char *buf, unsigned int bufsize, IPDatagram *dest);
};

#endif

