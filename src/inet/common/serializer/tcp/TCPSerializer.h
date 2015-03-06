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

#ifndef __INET_TCPSERIALIZER_H
#define __INET_TCPSERIALIZER_H

#include "inet/common/serializer/headers/defs.h"
#include "inet/common/serializer/SerializerBase.h"
#include "inet/common/serializer/tcp/headers/tcphdr.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet {

//forward declarations:
namespace tcp { class TCPSegment; }

namespace serializer {

/**
 * Converts between TCPSegment and binary (network byte order) TCP header.
 */
class TCPSerializer : public SerializerBase
{
  protected:
    virtual void serialize(const cPacket *pkt, Buffer &b, Context& context) override;
    virtual cPacket* deserialize(Buffer &b, Context& context) override;

  public:
    TCPSerializer(const char *name = nullptr) : SerializerBase(name) {}

    /**
     * Puts a packet sniffed from the wire into a TCPSegment.
     */
    tcp::TCPSegment *parse(const unsigned char *srcbuf, unsigned int bufsize, bool withBytes);

    /**
     * Calculate checksum with pseudo header.
     */
    static uint16_t checksum(const void *addr, unsigned int count, const void *addr2, unsigned int count2);
};

} // namespace serializer

} // namespace inet

#endif // ifndef __INET_TCPSERIALIZER_H

