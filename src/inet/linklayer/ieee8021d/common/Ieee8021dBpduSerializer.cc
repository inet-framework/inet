//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021d/common/Ieee8021dBpduSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ieee8021d/common/Ieee8021dBpdu_m.h"

namespace inet {

Register_Serializer(BpduBase, Ieee8021dBpduSerializer);
Register_Serializer(BpduCfg, Ieee8021dBpduSerializer);
Register_Serializer(BpduTcn, Ieee8021dBpduSerializer);

void Ieee8021dBpduSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& bpduBase = staticPtrCast<const BpduBase>(chunk);
    stream.writeUint16Be(bpduBase->getProtocolIdentifier());
    stream.writeUint8(bpduBase->getProtocolVersionIdentifier());
    stream.writeUint8(bpduBase->getBpduType());
    switch (bpduBase->getBpduType()) {
        case BPDU_CFG: {
            const auto& bpdu = CHK(dynamicPtrCast<const BpduCfg>(chunk));
            break;
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
            stream.writeUint16Be(bpdu->getMessageAge().dbl() * 256);
            stream.writeUint16Be(bpdu->getMaxAge().dbl() * 256);
            stream.writeUint16Be(bpdu->getHelloTime().dbl() * 256);
            stream.writeUint16Be(bpdu->getForwardDelay().dbl() * 256);
        }
        case BPDU_TCN: {
            const auto& bpdu = CHK(dynamicPtrCast<const BpduTcn>(chunk));
            (void)bpdu; // unused
            break;
        }
        default:
            throw cRuntimeError("Unknown BPDU TYPE value: %d", static_cast<int>(bpduBase->getBpduType()));
    }
}

const Ptr<Chunk> Ieee8021dBpduSerializer::deserialize(MemoryInputStream& stream) const
{
    auto protocolIdentifier = static_cast<BpduProtocolIdentifier>(stream.readUint16Be());
    auto protocolVersionIdentifier = static_cast<BpduProtocolVersionIdentifier>(stream.readUint8());
    BpduType bpduType = static_cast<BpduType>(stream.readUint8());

    switch (bpduType) {
        case BPDU_CFG: {
            auto bpdu = makeShared<BpduCfg>();
            bpdu->setProtocolIdentifier(protocolIdentifier);
            bpdu->setProtocolVersionIdentifier(protocolVersionIdentifier);
            bpdu->setBpduType(bpduType);
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
            bpdu->setMessageAge(SimTime(stream.readUint16Be() / 256.0));
            bpdu->setMaxAge(SimTime(stream.readUint16Be() / 256.0));
            bpdu->setHelloTime(SimTime(stream.readUint16Be() / 256.0));
            bpdu->setForwardDelay(SimTime(stream.readUint16Be() / 256.0));
            return bpdu;
        }
        case BPDU_TCN: {
            auto bpdu = makeShared<BpduTcn>();
            bpdu->setProtocolIdentifier(protocolIdentifier);
            bpdu->setProtocolVersionIdentifier(protocolVersionIdentifier);
            bpdu->setBpduType(bpduType);
            return bpdu;
        }
        default:
            throw cRuntimeError("Unknown BPDU TYPE value: %d", static_cast<int>(bpduType));
    }
}

} // namespace inet

