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
    stream.writeBit(bpdu->getBpduFlags().tcaFlag);
    stream.writeNBitsOfUint64Be(bpdu->getBpduFlags().reserved, 6);
    stream.writeBit(bpdu->getBpduFlags().tcFlag);
    stream.writeUint16Be(bpdu->getRootIdentifier().rootPriority);
    stream.writeMacAddress(bpdu->getRootIdentifier().rootAddress);
    stream.writeUint32Be(bpdu->getRootPathCost());
    stream.writeUint16Be(bpdu->getBridgeIdentifier().bridgePriority);
    stream.writeMacAddress(bpdu->getBridgeIdentifier().bridgeAddress);
    stream.writeByte(bpdu->getPortIdentifier().portPriority);
    stream.writeByte(bpdu->getPortIdentifier().portNum);
    stream.writeUint16Be(bpdu->getMessageAge().inUnit(SIMTIME_S) * 256);
    stream.writeUint16Be(bpdu->getMaxAge().inUnit(SIMTIME_S) * 256);
    stream.writeUint16Be(bpdu->getHelloTime().inUnit(SIMTIME_S) * 256);
    stream.writeUint16Be(bpdu->getForwardDelay().inUnit(SIMTIME_S) * 256);
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

    BpduFlags bpduFlags;
    bpduFlags.tcaFlag = stream.readBit();
    bpduFlags.reserved = stream.readNBitsToUint64Be(6);
    bpduFlags.tcFlag = stream.readBit();
    bpdu->setBpduFlags(bpduFlags);

    RootIdentifier rootId;
    rootId.rootPriority = stream.readUint16Be();
    rootId.rootAddress = stream.readMacAddress();
    bpdu->setRootIdentifier(rootId);

    bpdu->setRootPathCost(stream.readUint32Be());

    BridgeIdentifier bridgeId;
    bridgeId.bridgePriority = stream.readUint16Be();
    bridgeId.bridgeAddress = stream.readMacAddress();
    bpdu->setBridgeIdentifier(bridgeId);

    PortIdentifier portId;
    portId.portPriority = stream.readByte();
    portId.portNum = stream.readByte();
    bpdu->setPortIdentifier(portId);

    bpdu->setMessageAge(SimTime(stream.readUint16Be() / 256, SIMTIME_S));
    bpdu->setMaxAge(SimTime(stream.readUint16Be() / 256, SIMTIME_S));
    bpdu->setHelloTime(SimTime(stream.readUint16Be() / 256, SIMTIME_S));
    bpdu->setForwardDelay(SimTime(stream.readUint16Be() / 256, SIMTIME_S));
    // because of the KLUDGE in Rstp.cc (line 593) padding is added
    while (B(stream.getRemainingLength()) > B(0))
        stream.readByte();
    return bpdu;
}

} // namespace inet
