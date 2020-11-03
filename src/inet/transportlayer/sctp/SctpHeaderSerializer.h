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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef __INET_SCTPHEADERSERIALIZER_H
#define __INET_SCTPHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/transportlayer/sctp/SctpHeader.h"

namespace inet {
namespace sctp {

/**
 * Converts between SCTPMessage and binary (network byte order) SCTP header.
 */
class INET_API SctpHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
  //  virtual void serialize(const cPacket *pkt, Buffer &b, Context& context) override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
   // virtual cPacket* deserialize(const Buffer &b, Context& context) override;

  public:
    SctpHeaderSerializer(const char *name = nullptr) : FieldsChunkSerializer() {}

    /**
     * Serializes an SCTPMessage for transmission on the wire.
     * The checksum is NOT filled in. (The kernel does that when sending
     * the frame over a raw socket.)
     * Returns the length of data written into buffer.
     */
   // int32_t serialize(const SctpHeader *msg, uint8_t *buf, uint32_t bufsize);

    /**
     * Puts a packet sniffed from the wire into an SCTPMessage.
     */
  //  void parse(const uint8_t *buf, uint32_t bufsize, SctpHeader *dest);

    static uint32_t checksum(const uint8_t *buf, uint32_t len);
    static void hmacSha1(const uint8_t *buf, uint32_t buflen, const uint8_t *key, uint32_t keylen, uint8_t *digest);
    void calculateSharedKey();
    bool compareRandom();

  private:
    static unsigned char keyVector[512];
    static unsigned int sizeKeyVector;
    static unsigned char peerKeyVector[512];
    static unsigned int sizePeerKeyVector;
    static unsigned char sharedKey[512];
};

} // namespace sctp
} // namespace inet

#endif

