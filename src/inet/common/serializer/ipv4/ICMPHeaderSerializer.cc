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

#include "inet/applications/pingapp/PingPayload_m.h"
#include "inet/common/packet/SerializerRegistry.h"
#include "inet/common/serializer/ipv4/ICMPHeaderSerializer.h"
#include "inet/networklayer/ipv4/ICMPHeader_m.h"

namespace inet {

namespace serializer {

Register_Serializer(ICMPHeader, ICMPHeaderSerializer);
Register_Serializer(PingPayload, PingPayloadSerializer);

void ICMPHeaderSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& icmpHeader = std::static_pointer_cast<const ICMPHeader>(chunk);
    stream.writeByte(icmpHeader->getType());
    switch (icmpHeader->getType()) {
        case ICMP_ECHO_REQUEST:
            stream.writeByte(icmpHeader->getCode());
            stream.writeUint16(icmpHeader->getChksum());
            break;
        case ICMP_ECHO_REPLY:
            stream.writeByte(icmpHeader->getCode());
            stream.writeUint16(icmpHeader->getChksum());
            break;
        case ICMP_DESTINATION_UNREACHABLE:
            stream.writeByte(icmpHeader->getCode());
            stream.writeUint16(icmpHeader->getChksum());
            stream.writeUint16(0);   // unused
            stream.writeUint16(0);   // next hop MTU
            break;
        case ICMP_TIME_EXCEEDED:
            stream.writeByte(ICMP_TIMXCEED_INTRANS);
            stream.writeUint16(icmpHeader->getChksum());
            stream.writeUint32(0);   // unused
            break;
        default:
            throw cRuntimeError("Can not serialize ICMP packet: type %d  not supported.", icmpHeader->getType());
    }
}

std::shared_ptr<Chunk> ICMPHeaderSerializer::deserialize(ByteInputStream& stream) const
{
    auto icmpHeader = std::make_shared<ICMPHeader>();
    uint8_t type = stream.readByte();
    icmpHeader->setType(type);
    icmpHeader->setCode(stream.readByte());
    icmpHeader->setChksum(stream.readUint16());
    switch (type) {
        case ICMP_ECHO_REQUEST:
            icmpHeader->setChunkLength(byte(4));
            break;
        case ICMP_ECHO_REPLY:
            icmpHeader->setChunkLength(byte(4));
            break;
        case ICMP_DESTINATION_UNREACHABLE:
            stream.readUint16();   // unused
            stream.readUint16();   // next hop MTU
            icmpHeader->setChunkLength(byte(8));
            break;
        case ICMP_TIME_EXCEEDED:
            stream.readUint32();   // unused
            icmpHeader->setChunkLength(byte(8));
            break;
        default:
            EV_ERROR << "Can not parse ICMP packet: type " << type << " not supported.";
            icmpHeader->markImproperlyRepresented();
            break;
    }
    return icmpHeader;
}

void PingPayloadSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
//    const auto& pp = std::static_pointer_cast<const PingPayload>(chunk);
//    stream.writeUint16(pp->getOriginatorId());
//    stream.writeUint16(pp->getSeqNo());
//    unsigned int datalen = pp->getDataArraySize();
//    for (unsigned int i = 0; i < datalen; i++)
//        stream.writeByte(pp->getData(i));
//    datalen = (pp->getByteLength() - 4) - datalen;
//    stream.fillNBytes(datalen, 'a');
}

std::shared_ptr<Chunk> PingPayloadSerializer::deserialize(ByteInputStream& stream) const
{
    PingPayload *pp = new PingPayload();
    pp->setOriginatorId(stream.readUint16());
    uint16_t seqno = stream.readUint16();
    pp->setSeqNo(seqno);
    pp->setByteLength(4 + stream.getRemainingSize());
    pp->setDataArraySize(stream.getRemainingSize());
    for (unsigned int i = 0; stream.getRemainingSize() > 0; i++)
        pp->setData(i, stream.readByte());
    return nullptr;
}

} // namespace serializer

} // namespace inet

