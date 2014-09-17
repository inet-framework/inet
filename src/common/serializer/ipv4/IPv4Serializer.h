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

#ifndef __INET_IPV4SERIALIZER_H
#define __INET_IPV4SERIALIZER_H

#include "inet/networklayer/ipv4/IPv4Datagram.h"

namespace inet {

namespace serializer {

/**
 * Converts between IPv4Datagram and binary (network byte order) IPv4 header.
 */
class IPv4Serializer
{
  public:
    IPv4Serializer() {}

    /**
     * Serializes an IPv4Datagram for transmission on the wire.
     * The checksum is set to 0 when hasCalcChkSum is false. (The kernel does that when sending
     * the frame over a raw socket.)
     * When hasCalcChkSum is true, then calculating checksum.
     * Returns the length of data written into buffer.
     */
    int serialize(const IPv4Datagram *dgram, unsigned char *buf, unsigned int bufsize, bool hasCalcChkSum = false);

    /**
     * Puts a packet sniffed from the wire into an IPv4Datagram. Does NOT
     * verify the checksum.
     */
    void parse(const unsigned char *buf, unsigned int bufsize, IPv4Datagram *dest);
};

} // namespace serializer

} // namespace inet

#endif // ifndef __INET_IPV4SERIALIZER_H

