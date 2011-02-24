//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen
// Copyright (C) 2009 Thomas Reschka
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

#ifndef __TCPSERIALIZER_H
#define __TCPSERIALIZER_H


#include "IPvXAddress.h"

#include "headers/defs.h"

#include "headers/tcp.h"

//forward declarations:
class TCPSegment;

/**
 * Converts between TCPSegment and binary (network byte order) TCP header.
 */
class TCPSerializer
{
    public:
        TCPSerializer() {}

        /**
         * Serializes a TCPSegment for transmission on the wire.
         * The checksum is NOT filled in.
         * Returns the length of data written into buffer.
         * TODO msg why not a const reference?
         */
        int serialize(const TCPSegment *source, unsigned char *destbuf, unsigned int bufsize);

        /**
         * Serializes a TCPSegment for transmission on the wire.
         * The checksum is NOT filled in. (The kernel does that when sending
         * the frame over a raw socket.)
         * Returns the length of data written into buffer.
         * TODO msg why not a const reference?
         * TODO pseudoheader vs IPv6, pseudoheder.len should calculated by the serialize(), etc
         */
        int serialize(const TCPSegment *source, unsigned char *destbuf, unsigned int bufsize,
                const IPvXAddress &srcIp, const IPvXAddress &destIp);

        /**
         * Puts a packet sniffed from the wire into a TCPSegment.
         * TODO dest why not reference?
         */
        void parse(const unsigned char *srcbuf, unsigned int bufsize, TCPSegment *dest,
                bool withBytes);

        /**
         * Calculate checksum with pseudo header.
         */
        static uint16_t checksum(const void *addr, unsigned int count,
                const IPvXAddress &srcIp, const IPvXAddress &destIp);
};

#endif
