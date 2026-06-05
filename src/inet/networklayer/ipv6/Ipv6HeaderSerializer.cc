//
// Copyright (C) 2013 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv6/Ipv6HeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"

#if defined(_MSC_VER)
#undef s_addr /* MSVC #definition interferes with us */
#endif // if defined(_MSC_VER)

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h> // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

// This in_addr field is defined as a macro in Windows and Solaris, which interferes with us
#undef s_addr

namespace inet {

Register_Serializer(Ipv6Header, Ipv6HeaderSerializer);

void Ipv6HeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& ipv6Header = staticPtrCast<const Ipv6Header>(chunk);
    stream.writeUint4(ipv6Header->getVersion());
    stream.writeUint8(ipv6Header->getTrafficClass());
    stream.writeNBitsOfUint64Be(ipv6Header->getFlowLabel(), 20);
    stream.writeUint16Be(ipv6Header->getPayloadLength().get<B>());
    stream.writeByte(ipv6Header->getProtocolId());
    stream.writeByte(ipv6Header->getHopLimit());
    stream.writeIpv6Address(ipv6Header->getSrcAddress());
    stream.writeIpv6Address(ipv6Header->getDestAddress());
}

const Ptr<Chunk> Ipv6HeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ipv6Header = makeShared<Ipv6Header>();
    ipv6Header->setVersion(stream.readUint4());
    if (ipv6Header->getVersion() != 6)
        ipv6Header->markIncorrect();
    ipv6Header->setTrafficClass(stream.readUint8());
    ipv6Header->setFlowLabel(stream.readNBitsToUint64Be(20));
    ipv6Header->setPayloadLength(B(stream.readUint16Be()));
    ipv6Header->setProtocolId(static_cast<IpProtocolId>(stream.readByte()));
    ipv6Header->setHopLimit(stream.readByte());
    ipv6Header->setSrcAddress(stream.readIpv6Address());
    ipv6Header->setDestAddress(stream.readIpv6Address());
    return ipv6Header;
}

} // namespace inet

