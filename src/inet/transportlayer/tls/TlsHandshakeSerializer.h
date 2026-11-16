// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TLSHANDSHAKESERIALIZER_H
#define INET_TLSHANDSHAKESERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/transportlayer/tls/TlsClientHello_m.h"

namespace inet::tls {

class INET_API TlsHandshakeSerializer : public FieldsChunkSerializer
{
  protected:
    void serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    const Ptr<Chunk> deserializeFields(MemoryInputStream& stream, const std::type_info& typeInfo) const override;

  public:
    // DTLS inserts legacy_cookie between these two portions of ClientHello.
    static void writeClientPrefix(MemoryOutputStream& stream, const Ptr<const TlsClientHello>& hello);
    static void writeClientSuffix(MemoryOutputStream& stream, const Ptr<const TlsClientHello>& hello);
    static void readClientPrefix(MemoryInputStream& stream, const Ptr<TlsClientHello>& hello);
    static void readClientSuffix(MemoryInputStream& stream, const Ptr<TlsClientHello>& hello);
};

} // namespace inet::tls
#endif
