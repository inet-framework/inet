//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
//    virtual void serialize(const cPacket *pkt, Buffer &b, Context& context) override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
//    virtual cPacket* deserialize(const Buffer &b, Context& context) override;

  public:
    SctpHeaderSerializer(const char *name = nullptr) : FieldsChunkSerializer() {}

    /**
     * Serializes an SCTPMessage for transmission on the wire.
     * The checksum is NOT filled in. (The kernel does that when sending
     * the frame over a raw socket.)
     * Returns the length of data written into buffer.
     */
//    int32_t serialize(const SctpHeader *msg, uint8_t *buf, uint32_t bufsize);

    /**
     * Puts a packet sniffed from the wire into an SCTPMessage.
     */
//    void parse(const uint8_t *buf, uint32_t bufsize, SctpHeader *dest);

    static uint32_t checksum(const uint8_t *buf, uint32_t len);
    static void hmacSha1(const uint8_t *buf, uint32_t buflen, const uint8_t *key, uint32_t keylen, uint8_t *digest);
    void calculateSharedKey();
    bool compareRandom();
    static int getKeysHandle();

  public:
    struct Keys {
        unsigned char keyVector[512];
        unsigned int sizeKeyVector;
        unsigned char peerKeyVector[512];
        unsigned int sizePeerKeyVector;
        unsigned char sharedKey[512];
    };
  private:
    Keys& keys = getSimulationOrSharedDataManager()->getSharedVariable<Keys>(getKeysHandle());
};

} // namespace sctp
} // namespace inet

#endif

