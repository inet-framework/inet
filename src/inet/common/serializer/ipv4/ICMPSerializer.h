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

#ifndef __INET_ICMPSERIALIZER_H
#define __INET_ICMPSERIALIZER_H

#include "inet/networklayer/ipv4/ICMPMessage_m.h"

namespace inet {

namespace serializer {

/**
 * Converts between ICMPMessage and binary (network byte order) ICMP header.
 */
class ICMPSerializer
{
  public:
    ICMPSerializer() {}

    /**
     * Serializes an ICMPMessage for transmission on the wire.
     * Returns the length of data written into buffer.
     */
    int serialize(const ICMPMessage *pkt, unsigned char *buf, unsigned int bufsize);

    /**
     * Puts a packet sniffed from the wire into an ICMPMessage.
     */
    void parse(const unsigned char *buf, unsigned int bufsize, ICMPMessage *pkt);
};

} // namespace serializer

} // namespace inet

#endif // ifndef __INET_ICMPSERIALIZER_H

