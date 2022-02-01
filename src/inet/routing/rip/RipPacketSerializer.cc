//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/rip/RipPacketSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/routing/rip/RipPacket_m.h"

namespace inet {

// TODO
// The inet::Rip uses RipPacket and RipEntry for IPv4 (RIPv2, see RFC 1058)
// and for IPv6 (RIPng, see RFC 2080).
// The serializer accepts only RFC1058 packets with IPv4 addresses.

Register_Serializer(RipPacket, RipPacketSerializer);

void RipPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& ripPacket = staticPtrCast<const RipPacket>(chunk);

    stream.writeUint8(ripPacket->getCommand());
    stream.writeUint8(2); // RIP version

    int numEntries = ripPacket->getEntryArraySize();
    stream.writeUint16Be(numEntries);

    // iterate over each entry and write to stream
    for (int i = 0; i < numEntries; ++i) {
        const RipEntry& entry = ripPacket->getEntry(i);
        stream.writeUint16Be(entry.addressFamilyId);
        stream.writeUint16Be(entry.routeTag);
        stream.writeIpv4Address(entry.address.toIpv4());
        stream.writeIpv4Address(Ipv4Address::makeNetmask(entry.prefixLength));
        stream.writeIpv4Address(entry.nextHop.toIpv4());
        stream.writeUint32Be(entry.metric);
    }
}

const Ptr<Chunk> RipPacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ripPacket = makeShared<RipPacket>();

    ripPacket->setCommand((inet::RipCommand)stream.readUint8());
    int ripVer = stream.readUint8();
    if (ripVer != 2) {
        // TODO add RIP v1 support
        ripPacket->markIncorrect();
    }

    int numEntries = stream.readUint16Be();
    ripPacket->setEntryArraySize(numEntries);

    for (int i = 0; i < numEntries; ++i) {

        RipEntry entry = {};

        entry.addressFamilyId = (inet::RipAf)stream.readUint16Be();
        // TODO Valid addressFamilyId values: 0, 2, 0xFFFF
        // 0 and 2 means IPv4, 0xFFFF means Authentication packet
        entry.routeTag = stream.readUint16Be();
        entry.address = stream.readIpv4Address();
        Ipv4Address netmask = stream.readIpv4Address();
        entry.prefixLength = netmask.getNetmaskLength();
        if (netmask != Ipv4Address::makeNetmask(entry.prefixLength))
            ripPacket->markIncorrect(); // netmask can not be converted into prefixLength       //TODO should replace prefixLength to netmask
        entry.nextHop = stream.readIpv4Address();
        entry.metric = stream.readUint32Be();

        ripPacket->setEntry(i, entry);
    }

    return ripPacket;
}

} // namespace inet

