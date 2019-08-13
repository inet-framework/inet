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
#include "inet/linklayer/ieee8021d/common/Ieee8021dBpdu_m.h"
#include "inet/linklayer/ieee8021d/common/Ieee8021dBpduSerializer.h"

namespace inet {

Register_Serializer(Bpdu, Ieee8021dBpduSerializer);

void Ieee8021dBpduSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    B startPos = B(stream.getLength());
    const auto& bpdu = staticPtrCast<const Bpdu>(chunk);
    stream.writeUint16Be(bpdu->getProtocolIdentifier());
    stream.writeByte(bpdu->getProtocolVersionIdentifier());
    stream.writeByte(bpdu->getBpduType());
    if (bpdu->getBpduType() == 0) {
        stream.writeBit(bpdu->getTcaFlag());
        stream.writeNBitsOfUint64Be(bpdu->getReserved(), 6);
        stream.writeBit(bpdu->getTcFlag());
        stream.writeUint16Be(bpdu->getRootPriority());
        stream.writeMacAddress(bpdu->getRootAddress());
        stream.writeUint32Be(bpdu->getRootPathCost());
        stream.writeUint16Be(bpdu->getBridgePriority());
        stream.writeMacAddress(bpdu->getBridgeAddress());
        stream.writeByte(bpdu->getPortPriority());
        stream.writeByte(bpdu->getPortNum());
        stream.writeUint16Be(bpdu->getMessageAge().inUnit(SIMTIME_S) * 256);
        stream.writeUint16Be(bpdu->getMaxAge().inUnit(SIMTIME_S) * 256);
        stream.writeUint16Be(bpdu->getHelloTime().inUnit(SIMTIME_S) * 256);
        stream.writeUint16Be(bpdu->getForwardDelay().inUnit(SIMTIME_S) * 256);
    }
    // because of the KLUDGE in Rstp.cc (line 593) padding is added
    while (B(stream.getLength()) - startPos < B(bpdu->getChunkLength()))
        stream.writeByte('?');
}

const Ptr<Chunk> Ieee8021dBpduSerializer::deserialize(MemoryInputStream& stream) const
{
    auto bpdu = makeShared<Bpdu>();
    bpdu->setProtocolIdentifier(stream.readUint16Be());
    bpdu->setProtocolVersionIdentifier(stream.readByte());
    bpdu->setBpduType(stream.readByte());
    if (bpdu->getBpduType() == 0) {
        bpdu->setTcaFlag(stream.readBit());
        bpdu->setReserved(stream.readNBitsToUint64Be(6));
        bpdu->setTcFlag(stream.readBit());
        bpdu->setRootPriority(stream.readUint16Be());
        bpdu->setRootAddress(stream.readMacAddress());
        bpdu->setRootPathCost(stream.readUint32Be());
        bpdu->setBridgePriority(stream.readUint16Be());
        bpdu->setBridgeAddress(stream.readMacAddress());
        bpdu->setPortPriority(stream.readByte());
        bpdu->setPortNum(stream.readByte());
        bpdu->setMessageAge(SimTime(stream.readUint16Be() / 256, SIMTIME_S));
        bpdu->setMaxAge(SimTime(stream.readUint16Be() / 256, SIMTIME_S));
        bpdu->setHelloTime(SimTime(stream.readUint16Be() / 256, SIMTIME_S));
        bpdu->setForwardDelay(SimTime(stream.readUint16Be() / 256, SIMTIME_S));
    }
    // because of the KLUDGE in Rstp.cc (line 593) padding is added
    while (B(stream.getRemainingLength()) > B(0))
        stream.readByte();
    return bpdu;
}

} // namespace inet

