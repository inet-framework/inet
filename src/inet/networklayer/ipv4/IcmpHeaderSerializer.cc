//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen, Andras Varga
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

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#include "inet/networklayer/ipv4/IcmpHeaderSerializer.h"

namespace inet {

Register_Serializer(IcmpHeader, IcmpHeaderSerializer);
Register_Serializer(IcmpEchoRequest, IcmpHeaderSerializer);
Register_Serializer(IcmpEchoReply, IcmpHeaderSerializer);

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
            stream.writeUint16Be(0);   // unused
            stream.writeUint16Be(0);   // next hop MTU
            break;
        case ICMP_TIME_EXCEEDED:
            stream.writeUint32Be(0);   // unused
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
            stream.readUint16Be();   // unused
            stream.readUint16Be();   // next hop MTU
            break;
        case ICMP_TIME_EXCEEDED:
            stream.readUint32Be();   // unused
            break;
        default:
            EV_ERROR << "Can not parse ICMP packet: type " << type << " not supported.";
            icmpHeader->markImproperlyRepresented();
            break;
    }
    return icmpHeader;
}

} // namespace inet

