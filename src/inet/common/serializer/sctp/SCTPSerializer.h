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

#ifndef __INET_SCTPSERIALIZER_H
#define __INET_SCTPSERIALIZER_H

#include "inet/common/serializer/SerializerBase.h"
#include "inet/transportlayer/sctp/SCTPMessage.h"

namespace inet {

namespace serializer {

/**
 * Converts between SCTPMessage and binary (network byte order) SCTP header.
 */
class INET_API SCTPSerializer : public SerializerBase
{
  protected:
    virtual void serialize(const cPacket *pkt, Buffer &b, Context& context) override;
    virtual cPacket* deserialize(const Buffer &b, Context& context) override;

  public:
    SCTPSerializer(const char *name = nullptr) : SerializerBase(name) {}

    /**
     * Serializes an SCTPMessage for transmission on the wire.
     * The checksum is NOT filled in. (The kernel does that when sending
     * the frame over a raw socket.)
     * Returns the length of data written into buffer.
     */
    int32 serialize(const sctp::SCTPMessage *msg, uint8 *buf, uint32 bufsize);

    /**
     * Puts a packet sniffed from the wire into an SCTPMessage.
     */
    void parse(const uint8 *buf, uint32 bufsize, sctp::SCTPMessage *dest);

    static uint32 checksum(const uint8 *buf, register uint32 len);
    static void hmacSha1(const uint8 *buf, uint32 buflen, const uint8 *key, uint32 keylen, uint8 *digest);
    void calculateSharedKey();
    bool compareRandom();

  private:
    static unsigned char keyVector[512];
    static unsigned int sizeKeyVector;
    static unsigned char peerKeyVector[512];
    static unsigned int sizePeerKeyVector;
    static unsigned char sharedKey[512];
};

} // namespace serializer

} // namespace inet

#endif // ifndef __INET_SCTPSERIALIZER_H

