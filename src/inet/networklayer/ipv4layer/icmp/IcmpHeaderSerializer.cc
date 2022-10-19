//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen, Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4/IcmpHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/networklayer/ipv4/IcmpHeader_m.h"

namespace inet {

Register_Serializer(IcmpHeader, IcmpHeaderSerializer);
Register_Serializer(IcmpEchoRequest, IcmpHeaderSerializer);
Register_Serializer(IcmpEchoReply, IcmpHeaderSerializer);
Register_Serializer(IcmpPtb, IcmpHeaderSerializer);

void IcmpHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& icmpHeader = staticPtrCast<const IcmpHeader>(chunk);
    stream.writeByte(icmpHeader->getType());
    stream.writeByte(icmpHeader->getCode());
    stream.writeUint16Be(icmpHeader->getChksum());
    switch (icmpHeader->getType()) {
        case ICMP_ECHO_REQUEST: {
            const auto& icmpEchoRq = CHK(dynamicPtrCast<const IcmpEchoRequest>(chunk));
            stream.writeUint16Be(icmpEchoRq->getIdentifier());
            stream.writeUint16Be(icmpEchoRq->getSeqNumber());
            break;
        }
        case ICMP_ECHO_REPLY: {
            const auto& icmpEchoReply = CHK(dynamicPtrCast<const IcmpEchoReply>(chunk));
            stream.writeUint16Be(icmpEchoReply->getIdentifier());
            stream.writeUint16Be(icmpEchoReply->getSeqNumber());
            break;
        }
        case ICMP_DESTINATION_UNREACHABLE:
            if (icmpHeader->getCode() == ICMP_DU_FRAGMENTATION_NEEDED) {
                if (const auto& icmpPtb = dynamicPtrCast<const IcmpPtb>(chunk)) {
                    stream.writeUint16Be(icmpPtb->getUnused()); // unused
                    stream.writeUint16Be(icmpPtb->getMtu()); // next hop MTU
                }
                else
                    stream.writeUint32Be(0);
            }
            else
                stream.writeUint32Be(0); // unused
            break;
        case ICMP_TIME_EXCEEDED:
            stream.writeUint32Be(0); // unused
            break;
        default:
            throw cRuntimeError("Can not serialize ICMP packet: type %d  not supported.", icmpHeader->getType());
    }
}

const Ptr<Chunk> IcmpHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto icmpHeader = makeShared<IcmpHeader>();
    IcmpType type = static_cast<IcmpType>(stream.readByte());
    icmpHeader->setType(type);
    icmpHeader->setCode(stream.readByte());
    icmpHeader->setChksum(stream.readUint16Be());
    icmpHeader->setCrcMode(CRC_COMPUTED);
    switch (type) {
        case ICMP_ECHO_REQUEST: {
            auto echoRq = makeShared<IcmpEchoRequest>();
            echoRq->setType(type);
            echoRq->setCode(icmpHeader->getCode());
            echoRq->setChksum(icmpHeader->getChksum());
            echoRq->setCrcMode(CRC_COMPUTED);
            echoRq->setIdentifier(stream.readUint16Be());
            echoRq->setSeqNumber(stream.readUint16Be());
            icmpHeader = echoRq;
            break;
        }
        case ICMP_ECHO_REPLY: {
            auto echoReply = makeShared<IcmpEchoReply>();
            echoReply->setType(type);
            echoReply->setCode(icmpHeader->getCode());
            echoReply->setChksum(icmpHeader->getChksum());
            echoReply->setCrcMode(CRC_COMPUTED);
            echoReply->setIdentifier(stream.readUint16Be());
            echoReply->setSeqNumber(stream.readUint16Be());
            icmpHeader = echoReply;
            break;
        }
        case ICMP_DESTINATION_UNREACHABLE:
            if (icmpHeader->getCode() == ICMP_DU_FRAGMENTATION_NEEDED) {
                auto icmpPtb = makeShared<IcmpPtb>();
                icmpPtb->setType(type);
                icmpPtb->setCode(icmpHeader->getCode());
                icmpPtb->setChksum(icmpHeader->getChksum());
                icmpPtb->setCrcMode(CRC_COMPUTED);
                icmpPtb->setUnused(stream.readUint16Be());
                icmpPtb->setMtu(stream.readUint16Be());
                icmpHeader = icmpPtb;
            }
            else
                stream.readUint32Be(); // unused
            break;
        case ICMP_TIME_EXCEEDED:
            stream.readUint32Be(); // unused
            break;
        default:
            EV_ERROR << "Can not parse ICMP packet: type " << type << " not supported.";
            icmpHeader->markImproperlyRepresented();
            break;
    }
    return icmpHeader;
}

} // namespace inet

