//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/common/TransportPseudoHeaderSerializer.h"

#include "inet/common/Protocol.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/transportlayer/common/TransportPseudoHeader_m.h"

namespace inet {

Register_Serializer(TransportPseudoHeader, TransportPseudoHeaderSerializer);

void TransportPseudoHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    // FIXME ipv6, generic ????
    const auto& transportPseudoHeader = staticPtrCast<const TransportPseudoHeader>(chunk);
    auto nwProtId = transportPseudoHeader->getNetworkProtocolId();
    if (nwProtId == Protocol::ipv4.getId()) {
        ASSERT(transportPseudoHeader->getChunkLength() == B(12));
        stream.writeIpv4Address(transportPseudoHeader->getSrcAddress().toIpv4());
        stream.writeIpv4Address(transportPseudoHeader->getDestAddress().toIpv4());
        stream.writeByte(0);
        stream.writeByte(transportPseudoHeader->getProtocolId());
        stream.writeUint16Be(B(transportPseudoHeader->getPacketLength()).get());
    }
    else if (nwProtId == Protocol::ipv6.getId()) {
        ASSERT(transportPseudoHeader->getChunkLength() == B(40));
        stream.writeIpv6Address(transportPseudoHeader->getSrcAddress().toIpv6());
        stream.writeIpv6Address(transportPseudoHeader->getDestAddress().toIpv6());
        stream.writeUint32Be(B(transportPseudoHeader->getPacketLength()).get());
        stream.writeByte(0);
        stream.writeByte(0);
        stream.writeByte(0);
        stream.writeByte(transportPseudoHeader->getProtocolId());
    }
    else
        throw cRuntimeError("Unknown network protocol: %d", nwProtId);
}

const Ptr<Chunk> TransportPseudoHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    throw cRuntimeError("TransportPseudoHeader is not a valid deserializable data");
}

} // namespace inet

