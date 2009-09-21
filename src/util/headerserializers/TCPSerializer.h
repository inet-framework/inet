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

#ifndef __TCPSERIALIZER_H
#define __TCPSERIALIZER_H


#include "TCPSegment.h"

#include "headers/defs.h"
namespace INETFw // load headers into a namespace, to avoid conflicts with platform definitions of the same stuff
{
#include "headers/bsdint.h"
#include "headers/in.h"
#include "headers/in_systm.h"
#include "headers/tcp.h"
};

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>  // htonl, ntohl, ...
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

using namespace INETFw;

/**
 * Converts between TCPSegment and binary (network byte order) TCP header.
 */
class TCPSerializer
{
    public:
        TCPSerializer() {}

        /**
         * Serializes a TCPSegment for transmission on the wire.
         * The checksum is NOT filled in. (The kernel does that when sending
         * the frame over a raw socket.)
         * Returns the length of data written into buffer.
         */
        int serialize(TCPSegment *msg, unsigned char *buf, unsigned int bufsize, pseudoheader *pseudo);

        /**
         * Puts a packet sniffed from the wire into a TCPSegment.
         */
        //void parse(unsigned char *buf, unsigned int bufsize, TCPSegment *dest); // TODO implement

        static unsigned short checksum(unsigned char *addr, unsigned int count);
};

#endif

