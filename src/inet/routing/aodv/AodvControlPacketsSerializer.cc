//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/routing/aodv/AodvControlPackets_m.h"
#include "inet/routing/aodv/AodvControlPacketsSerializer.h"

namespace inet::aodv {

Register_Serializer(AodvControlPacket, AodvControlPacketsSerializer);

Register_Serializer(Rreq, AodvControlPacketsSerializer);
Register_Serializer(Rrep, AodvControlPacketsSerializer);
Register_Serializer(Rerr, AodvControlPacketsSerializer);
Register_Serializer(RrepAck, AodvControlPacketsSerializer);

void AodvControlPacketsSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& aodvControlPacket = staticPtrCast<const AodvControlPacket>(chunk);
    switch (aodvControlPacket->getPacketType()) {
        case RREQ: {
            const auto& aodvRreq = CHK(dynamicPtrCast<const Rreq>(chunk));
            stream.writeByte(aodvRreq->getPacketType());
            stream.writeBit(aodvRreq->getJoinFlag());
            stream.writeBit(aodvRreq->getRepairFlag());
            stream.writeBit(aodvRreq->getGratuitousRREPFlag());
            stream.writeBit(aodvRreq->getDestOnlyFlag());
            stream.writeBit(aodvRreq->getUnknownSeqNumFlag());
            stream.writeNBitsOfUint64Be(aodvRreq->getReserved(), 11);
            stream.writeByte(aodvRreq->getHopCount());
            stream.writeUint32Be(aodvRreq->getRreqId());
            stream.writeIpv4Address(aodvRreq->getDestAddr().toIpv4());
            stream.writeUint32Be(aodvRreq->getDestSeqNum());
            stream.writeIpv4Address(aodvRreq->getOriginatorAddr().toIpv4());
            stream.writeUint32Be(aodvRreq->getOriginatorSeqNum());
            ASSERT(aodvRreq->getChunkLength() == B(24));
            break;
        }
        case RREP: {
            const auto& aodvRrep = CHK(dynamicPtrCast<const Rrep>(chunk));
            stream.writeByte(aodvRrep->getPacketType());
            stream.writeBit(aodvRrep->getRepairFlag());
            stream.writeBit(aodvRrep->getAckRequiredFlag());
            stream.writeNBitsOfUint64Be(aodvRrep->getReserved(), 9);
            stream.writeNBitsOfUint64Be(aodvRrep->getPrefixSize(), 5);
            stream.writeByte(aodvRrep->getHopCount());
            stream.writeIpv4Address(aodvRrep->getDestAddr().toIpv4());
            stream.writeUint32Be(aodvRrep->getDestSeqNum());
            stream.writeIpv4Address(aodvRrep->getOriginatorAddr().toIpv4());
            stream.writeUint32Be(aodvRrep->getLifeTime().inUnit(SIMTIME_MS));
            ASSERT(aodvRrep->getChunkLength() == B(20));
            break;
        }
        case RERR: {
            const auto& aodvRerr = CHK(dynamicPtrCast<const Rerr>(chunk));
            stream.writeByte(aodvRerr->getPacketType());
            stream.writeBit(aodvRerr->getNoDeleteFlag());
            stream.writeNBitsOfUint64Be(aodvRerr->getReserved(), 15);
            if(aodvRerr->getUnreachableNodesArraySize() == 0){
                throw cRuntimeError("Cannot serialize AODV control packet of type %d: DestCount must be at least 1.", aodvControlPacket->getPacketType());
            }

            stream.writeByte(aodvRerr->getUnreachableNodesArraySize());
            for(uint8_t index = 0; index < aodvRerr->getUnreachableNodesArraySize(); ++index){
                stream.writeIpv4Address(aodvRerr->getUnreachableNodes(aodvRerr->getUnreachableNodesArraySize() - (index + 1)).addr.toIpv4());
                stream.writeUint32Be(aodvRerr->getUnreachableNodes(aodvRerr->getUnreachableNodesArraySize() - (index + 1)).seqNum);
            }
            ASSERT(aodvRerr->getChunkLength() == B(4 + aodvRerr->getUnreachableNodesArraySize() * 8));
            break;
        }
        case RREPACK: {
            const auto& aodvRrepAck = CHK(dynamicPtrCast<const RrepAck>(chunk));
            stream.writeByte(aodvRrepAck->getPacketType());
            stream.writeByte(aodvRrepAck->getReserved());
            ASSERT(aodvRrepAck->getChunkLength() == B(2));
            break;
        }
        default: {
            throw cRuntimeError("Cannot serialize AODV control packet: type %d not supported.", aodvControlPacket->getPacketType());
        }
    }
}

const Ptr<Chunk> AodvControlPacketsSerializer::deserialize(MemoryInputStream& stream) const
{
    auto aodvControlPacket = makeShared<AodvControlPacket>();
    AodvControlPacketType packetType = static_cast<AodvControlPacketType>(stream.readByte());
    switch (packetType) {
        case RREQ: {
            auto aodvRreq = makeShared<Rreq>();
            aodvRreq->setPacketType(packetType);
            aodvRreq->setJoinFlag(stream.readBit());
            aodvRreq->setRepairFlag(stream.readBit());
            aodvRreq->setGratuitousRREPFlag(stream.readBit());
            aodvRreq->setDestOnlyFlag(stream.readBit());
            aodvRreq->setUnknownSeqNumFlag(stream.readBit());
            aodvRreq->setReserved(stream.readNBitsToUint64Be(11));
            aodvRreq->setHopCount(stream.readByte());
            aodvRreq->setRreqId(stream.readUint32Be());
            aodvRreq->setDestAddr(L3Address(stream.readIpv4Address()));
            aodvRreq->setDestSeqNum(stream.readUint32Be());
            aodvRreq->setOriginatorAddr(L3Address(stream.readIpv4Address()));
            aodvRreq->setOriginatorSeqNum(stream.readUint32Be());
            return aodvRreq;
        }
        case RREP: {
            auto aodvRrep = makeShared<Rrep>();
            aodvRrep->setPacketType(packetType);
            aodvRrep->setRepairFlag(stream.readBit());
            aodvRrep->setAckRequiredFlag(stream.readBit());
            aodvRrep->setReserved(stream.readNBitsToUint64Be(9));
            aodvRrep->setPrefixSize(stream.readNBitsToUint64Be(5));
            aodvRrep->setHopCount(stream.readByte());
            aodvRrep->setDestAddr(L3Address(stream.readIpv4Address()));
            aodvRrep->setDestSeqNum(stream.readUint32Be());
            aodvRrep->setOriginatorAddr(L3Address(stream.readIpv4Address()));
            aodvRrep->setLifeTime(SimTime(stream.readUint32Be(), SIMTIME_MS));
            return aodvRrep;
        }
        case RERR: {
            const auto& aodvRerr = makeShared<Rerr>();
            aodvRerr->setPacketType(packetType);
            aodvRerr->setNoDeleteFlag(stream.readBit());
            aodvRerr->setReserved(stream.readNBitsToUint64Be(15));
            aodvRerr->setUnreachableNodesArraySize(stream.readByte());
            if(aodvRerr->getUnreachableNodesArraySize() == 0)
                aodvRerr->markIncorrect();
            UnreachableNode node = UnreachableNode();
            for(uint8_t index = 0; index < aodvRerr->getUnreachableNodesArraySize(); ++index){
                node.addr = L3Address(stream.readIpv4Address());
                node.seqNum = stream.readUint32Be();
                aodvRerr->setUnreachableNodes(aodvRerr->getUnreachableNodesArraySize() - (index + 1), node);
            }
            aodvRerr->setChunkLength(B(4 + aodvRerr->getUnreachableNodesArraySize() * 8));
            return aodvRerr;
        }
        case RREPACK:{
            const auto& aodvRrepAck = makeShared<RrepAck>();
            aodvRrepAck->setPacketType(packetType);
            aodvRrepAck->setReserved(stream.readByte());
            return aodvRrepAck;
        }
        default: {
            aodvControlPacket->markIncorrect();
        }
    }
}

} // namespace inet::aodv


















