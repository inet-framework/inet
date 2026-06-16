//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/icmpv6/MldHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/networklayer/icmpv6/MldMessage_m.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h> // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

namespace inet {

Register_Serializer(MldMessage, MldHeaderSerializer);
Register_Serializer(MldQuery, MldHeaderSerializer);
Register_Serializer(MldReport, MldHeaderSerializer);
Register_Serializer(MldDone, MldHeaderSerializer);

void MldHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& msg = staticPtrCast<const MldMessage>(chunk);
    stream.writeByte(msg->getType());                      // type (1 byte)
    stream.writeByte(msg->getCode());                      // code (1 byte, always 0)
    stream.writeUint16Be(msg->getChksum());               // checksum (2 bytes)
    stream.writeUint16Be(msg->getMaxRespDelay());         // Maximum Response Delay (2 bytes, ms)
    stream.writeUint16Be(msg->getReserved());             // reserved (2 bytes)
    stream.writeIpv6Address(msg->getMulticastAddress()); // Multicast Address (16 bytes)
}

const Ptr<Chunk> MldHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    uint8_t type = stream.readByte();
    uint8_t code = stream.readByte();
    uint16_t chksum = stream.readUint16Be();
    uint16_t maxRespDelay = stream.readUint16Be();
    uint16_t reserved = stream.readUint16Be();
    Ipv6Address mcastAddr = stream.readIpv6Address();

    Ptr<MldMessage> msg;
    switch (type) {
        case ICMPv6_MLD_QUERY: {
            auto q = makeShared<MldQuery>();
            msg = q;
            break;
        }
        case ICMPv6_MLD_REPORT: {
            auto r = makeShared<MldReport>();
            msg = r;
            break;
        }
        case ICMPv6_MLD_DONE: {
            auto d = makeShared<MldDone>();
            msg = d;
            break;
        }
        default: {
            EV_ERROR << "MldHeaderSerializer: cannot parse MLD packet: type " << (int)type << " not supported\n";
            auto unknown = makeShared<MldMessage>();
            unknown->setChunkLength(B(24)); // 24 bytes already consumed; keep length in sync so later chunks parse correctly
            unknown->markIncorrect();
            return unknown;
        }
    }
    msg->setType(static_cast<Icmpv6Type>(type));
    msg->setCode(code);
    msg->setChksum(chksum);
    msg->setChecksumMode(CHECKSUM_COMPUTED);
    msg->setMaxRespDelay(maxRespDelay);
    msg->setReserved(reserved);
    msg->setMulticastAddress(mcastAddr);
    return msg;
}

} // namespace inet
