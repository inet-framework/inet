//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/pim/PimPacketSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/routing/pim/PimPacket_m.h"

namespace inet {

Register_Serializer(PimPacket, PimPacketSerializer);

Register_Serializer(PimHello, PimPacketSerializer);
Register_Serializer(PimJoinPrune, PimPacketSerializer);
Register_Serializer(PimAssert, PimPacketSerializer);
Register_Serializer(PimGraft, PimPacketSerializer);
Register_Serializer(PimStateRefresh, PimPacketSerializer);
Register_Serializer(PimRegister, PimPacketSerializer);
Register_Serializer(PimRegisterStop, PimPacketSerializer);

namespace {

// TODO EncodedAddress serializers/deserializers currently allow only IPv4 addresses with native encoding

void serializeEncodedUnicastAddress(MemoryOutputStream& stream, const EncodedUnicastAddress& addr)
{
    stream.writeByte(1); // Address Family: '1' represents the IPv4 address family
    stream.writeByte(0); // Encoding Type: '0' represents the native encoding of the Address Family.
    stream.writeIpv4Address(addr.unicastAddress.toIpv4());
}

void serializeEncodedGroupAddress(MemoryOutputStream& stream, const EncodedGroupAddress& addr)
{
    // Encoded-Group Address:
    stream.writeByte(1); // Address Family: '1' represents the IPv4 address family
    stream.writeByte(0); // Encoding Type: '0' represents the native encoding of the Address Family.
    stream.writeBit(addr.B);
    stream.writeNBitsOfUint64Be(addr.reserved, 6); // Reserved
    stream.writeBit(addr.Z); // FIXME Admin Scope [Z]one: indicates the group range is an admin scope zone.
    stream.writeByte(addr.maskLength); // Mask Length: 32 for IPv4 native encoding
    stream.writeIpv4Address(addr.groupAddress.toIpv4());
}

void serializeEncodedSourceAddress(MemoryOutputStream& stream, const EncodedSourceAddress& addr)
{
    stream.writeByte(1); // Address Family: '1' represents the IPv4 address family
    stream.writeByte(0); // Encoding Type: '0' represents the native encoding of the Address Family.
    stream.writeNBitsOfUint64Be(addr.reserved, 5); // Reserved
    stream.writeBit(addr.S);
    stream.writeBit(addr.W);
    stream.writeBit(addr.R);
    stream.writeByte(addr.maskLength); // Mask Length: 32 for IPv4 native encoding
    stream.writeIpv4Address(addr.sourceAddress.toIpv4());
}

void deserializeEncodedUnicastAddress(MemoryInputStream& stream, const Ptr<PimPacket> pimPacket, EncodedUnicastAddress& addr)
{
    // Encoded-Unicast Address
    uint8_t addressFamily = stream.readByte(); // Address Family: '1' represents the IPv4 address family
    uint8_t encodingType = stream.readByte(); // Encoding Type: '0' represents the native encoding of the Address Family.
    if (addressFamily != 1 || encodingType != 0)
        pimPacket->markIncorrect();
    addr.unicastAddress = stream.readIpv4Address();
}

void deserializeEncodedGroupAddress(MemoryInputStream& stream, const Ptr<PimPacket> pimPacket, EncodedGroupAddress& addr)
{
    // Encoded-Group Address:
    uint8_t addressFamily = stream.readByte(); // Address Family: '1' represents the IPv4 address family
    uint8_t encodingType = stream.readByte(); // Encoding Type: '0' represents the native encoding of the Address Family.
    addr.B = stream.readBit();
    addr.reserved = stream.readNBitsToUint64Be(6); // Reserved
    addr.Z = stream.readBit(); // FIXME Admin Scope [Z]one: indicates the group range is an admin scope zone.
    addr.maskLength = stream.readByte(); // Mask Length: 32 for IPv4 native encoding
    if (addressFamily != 1 || encodingType != 0 || addr.maskLength != 32)
        pimPacket->markIncorrect();
    addr.groupAddress = stream.readIpv4Address();
}

void deserializeEncodedSourceAddress(MemoryInputStream& stream, const Ptr<PimPacket> pimPacket, EncodedSourceAddress& addr)
{
    // Encoded-Source Address
    uint8_t addressFamily = stream.readByte(); // Address Family: '1' represents the IPv4 address family
    uint8_t encodingType = stream.readByte(); // Encoding Type: '0' represents the native encoding of the Address Family.
    addr.reserved = stream.readNBitsToUint64Be(5); // Reserved
    addr.S = stream.readBit();
    addr.W = stream.readBit();
    addr.R = stream.readBit();
    addr.maskLength = stream.readByte(); // Mask Length: 32 for IPv4 native encoding
    if (addressFamily != 1 || encodingType != 0 || addr.maskLength != 32)
        pimPacket->markIncorrect();
    addr.sourceAddress = stream.readIpv4Address();
}

} // namespace

void PimPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& pimPacket = staticPtrCast<const PimPacket>(chunk);
    // PIM header common to all PIM messages:
    // | PIM version (4 bits) | Type (4 bits) | Reserved (8 bits) | Checksum (16 bits) |
    stream.writeNBitsOfUint64Be(pimPacket->getVersion(), 4);
    stream.writeNBitsOfUint64Be(pimPacket->getType(), 4);
    stream.writeByte(pimPacket->getReserved());
    stream.writeUint16Be(pimPacket->getCrc());
    switch (pimPacket->getType()) {
        case Hello: {
            const auto& pimHello = staticPtrCast<const PimHello>(chunk);
            for (size_t i = 0; i < pimHello->getOptionsArraySize(); ++i) {
                stream.writeUint16Be(pimHello->getOptions(i)->getType()); // type
                switch (pimHello->getOptions(i)->getType()) {
                    case Holdtime: {
                        const HoldtimeOption *holdtimeOption = static_cast<const HoldtimeOption *>(pimHello->getOptions(i));
                        stream.writeUint16Be(2); // length
                        stream.writeUint16Be(holdtimeOption->getHoldTime());
                        break;
                    }
                    case LANPruneDelay: {
                        const LanPruneDelayOption *lanPruneDelayOption = static_cast<const LanPruneDelayOption *>(pimHello->getOptions(i));
                        stream.writeUint16Be(4); // length
                        stream.writeBit(0); // FIXME T bit missing
                        stream.writeNBitsOfUint64Be(lanPruneDelayOption->getPropagationDelay(), 15);
                        stream.writeUint16Be(lanPruneDelayOption->getOverrideInterval());
                        break;
                    }
                    case DRPriority: {
                        const DrPriorityOption *drPriorityOption = static_cast<const DrPriorityOption *>(pimHello->getOptions(i));
                        stream.writeUint16Be(4); // length
                        stream.writeUint32Be(drPriorityOption->getPriority());
                        break;
                    }
                    case GenerationID: {
                        const GenerationIdOption *generationIdOption = static_cast<const GenerationIdOption *>(pimHello->getOptions(i));
                        stream.writeUint16Be(4); // length
                        stream.writeUint32Be(generationIdOption->getGenerationID());
                        break;
                    }
                    default:
                        throw cRuntimeError("Cannot serialize Hello options: type %d not supported.", pimHello->getOptions(i)->getType());
                }
            }
            break;
        }
        case Register: {
            const auto& pimRegister = staticPtrCast<const PimRegister>(chunk);
            stream.writeBit(pimRegister->getB());
            stream.writeBit(pimRegister->getN());
            stream.writeNBitsOfUint64Be(0, 30); // Reserved
            break;
        }
        case RegisterStop: {
            const auto& pimRegisterStop = staticPtrCast<const PimRegisterStop>(chunk);
            serializeEncodedGroupAddress(stream, pimRegisterStop->getGroupAddress());
            serializeEncodedUnicastAddress(stream, pimRegisterStop->getSourceAddress());
            break;
        }
        case GraftAck:
        case Graft:
        case JoinPrune: {
            const auto& pimJoinPrune = staticPtrCast<const PimJoinPrune>(chunk);
            serializeEncodedUnicastAddress(stream, pimJoinPrune->getUpstreamNeighborAddress());
            stream.writeByte(pimJoinPrune->getReserved2());
            stream.writeByte(pimJoinPrune->getJoinPruneGroupsArraySize());
            if (pimPacket->getType() == Graft || pimPacket->getType() == GraftAck)
                stream.writeUint16Be(0);
            else
                stream.writeUint16Be(pimJoinPrune->getHoldTime());
            for (size_t i = 0; i < pimJoinPrune->getJoinPruneGroupsArraySize(); ++i) {
                auto& joinPruneGroups = pimJoinPrune->getJoinPruneGroups(i);
                serializeEncodedGroupAddress(stream, joinPruneGroups.getGroupAddress());
                stream.writeUint16Be(joinPruneGroups.getJoinedSourceAddressArraySize());
                stream.writeUint16Be(joinPruneGroups.getPrunedSourceAddressArraySize());
                for (size_t k = 0; k < joinPruneGroups.getJoinedSourceAddressArraySize(); ++k) {
                    serializeEncodedSourceAddress(stream, joinPruneGroups.getJoinedSourceAddress(k));
                }
                for (size_t k = 0; k < joinPruneGroups.getPrunedSourceAddressArraySize(); ++k) {
                    serializeEncodedSourceAddress(stream, joinPruneGroups.getPrunedSourceAddress(k));
                }
            }
            break;
        }
        case Assert: {
            const auto& pimAssert = staticPtrCast<const PimAssert>(chunk);
            serializeEncodedGroupAddress(stream, pimAssert->getGroupAddress());
            serializeEncodedUnicastAddress(stream, pimAssert->getSourceAddress());
            stream.writeBit(pimAssert->getR());
            stream.writeNBitsOfUint64Be(pimAssert->getMetricPreference(), 31);
            stream.writeUint32Be(pimAssert->getMetric());
            break;
        }
        case StateRefresh: {
            const auto& pimStateRefresh = staticPtrCast<const PimStateRefresh>(chunk);
            serializeEncodedGroupAddress(stream, pimStateRefresh->getGroupAddress());
            serializeEncodedUnicastAddress(stream, pimStateRefresh->getSourceAddress());
            serializeEncodedUnicastAddress(stream, pimStateRefresh->getOriginatorAddress());
            stream.writeBit(pimStateRefresh->getR()); // R: The Rendezvous Point Tree bit.  Set to 0 for PIM-DM.  Ignored upon receipt.
            stream.writeNBitsOfUint64Be(pimStateRefresh->getMetricPreference(), 31);
            stream.writeUint32Be(pimStateRefresh->getMetric());
            stream.writeByte(pimStateRefresh->getMaskLen());
            stream.writeByte(pimStateRefresh->getTtl());
            stream.writeBit(pimStateRefresh->getP());
            stream.writeBit(pimStateRefresh->getN());
            stream.writeBit(pimStateRefresh->getO());
            stream.writeNBitsOfUint64Be(pimStateRefresh->getReserved2(), 5);
            stream.writeByte(pimStateRefresh->getInterval());
            break;
        }
        default:
            throw cRuntimeError("Cannot serialize PIM packet: type %d not supported.", pimPacket->getType());
    }
}

const Ptr<Chunk> PimPacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto pimPacket = makeShared<PimPacket>();
    // PIM header common to all PIM messages:
    // | PIM version (4 bits) | Type (4 bits) | Reserved (8 bits) | Checksum (16 bits) |
    pimPacket->setVersion(stream.readNBitsToUint64Be(4));
    pimPacket->setType(static_cast<PimPacketType>(stream.readNBitsToUint64Be(4)));
    pimPacket->setReserved(stream.readByte());
    pimPacket->setCrc(stream.readUint16Be());
    pimPacket->setCrcMode(CRC_COMPUTED);
    B length = B(4); // header length
    switch (pimPacket->getType()) {
        case Hello: {
            auto pimHello = makeShared<PimHello>();
            pimHello->setType(pimPacket->getType());
            pimHello->setVersion(pimPacket->getVersion());
            pimHello->setReserved(pimPacket->getReserved());
            pimHello->setCrc(pimPacket->getCrc());
            pimHello->setCrcMode(pimPacket->getCrcMode());
            PimHelloOptionType type;
            size_t i = 0;
            while (stream.getRemainingLength().get() > 0) {
                type = static_cast<PimHelloOptionType>(stream.readUint16Be());
                switch (type) {
                    case Holdtime: {
                        uint16_t size = stream.readUint16Be();
                        if (size != 2)
                            pimHello->markIncorrect();
                        pimHello->setOptionsArraySize(++i);
                        HoldtimeOption *holdtimeOption = new HoldtimeOption();
                        holdtimeOption->setHoldTime(stream.readUint16Be());
                        pimHello->setOptions(i - 1, holdtimeOption);
                        length += B(6);
                        break;
                    }
                    case LANPruneDelay: {
                        uint16_t size = stream.readUint16Be();
                        bool T = stream.readBit(); // T bit
                        if (size != 4 || T)
                            pimHello->markIncorrect();
                        pimHello->setOptionsArraySize(++i);
                        LanPruneDelayOption *lanPruneDelayOption = new LanPruneDelayOption();
                        lanPruneDelayOption->setPropagationDelay(stream.readNBitsToUint64Be(15));
                        lanPruneDelayOption->setOverrideInterval(stream.readUint16Be());
                        pimHello->setOptions(i - 1, lanPruneDelayOption);
                        length += B(8);
                        break;
                    }
                    case DRPriority: {
                        uint16_t size = stream.readUint16Be();
                        if (size != 4)
                            pimHello->markIncorrect();
                        pimHello->setOptionsArraySize(++i);
                        DrPriorityOption *drPriorityOption = new DrPriorityOption();
                        drPriorityOption->setPriority(stream.readUint32Be());
                        pimHello->setOptions(i - 1, drPriorityOption);
                        length += B(8);
                        break;
                    }
                    case GenerationID: {
                        uint16_t size = stream.readUint16Be();
                        if (size != 4)
                            pimHello->markIncorrect();
                        pimHello->setOptionsArraySize(++i);
                        GenerationIdOption *generationIdOption = new GenerationIdOption();
                        generationIdOption->setGenerationID(stream.readUint32Be());
                        pimHello->setOptions(i - 1, generationIdOption);
                        length += B(8);
                        break;
                    }
                    default:
                        throw cRuntimeError("Cannot deserialize Hello options: type %d not supported.", type);
                }
            }
            pimHello->setChunkLength(length);
            return pimHello;
        }
        case Register: {
            auto pimRegister = makeShared<PimRegister>();
            pimRegister->setType(pimPacket->getType());
            pimRegister->setVersion(pimPacket->getVersion());
            pimRegister->setReserved(pimPacket->getReserved());
            pimRegister->setCrc(pimPacket->getCrc());
            pimRegister->setCrcMode(pimPacket->getCrcMode());
            pimRegister->setB(stream.readBit());
            pimRegister->setN(stream.readBit());
            stream.readNBitsToUint64Be(30); // Reserved
            length += B(4);
            pimRegister->setChunkLength(length);
            return pimRegister;
        }
        case RegisterStop: {
            auto pimRegisterStop = makeShared<PimRegisterStop>();
            pimRegisterStop->setType(pimPacket->getType());
            pimRegisterStop->setVersion(pimPacket->getVersion());
            pimRegisterStop->setReserved(pimPacket->getReserved());
            pimRegisterStop->setCrc(pimPacket->getCrc());
            pimRegisterStop->setCrcMode(pimPacket->getCrcMode());
            deserializeEncodedGroupAddress(stream, pimRegisterStop, pimRegisterStop->getGroupAddressForUpdate());
            length += ENCODED_GROUP_ADDRESS_LENGTH;
            deserializeEncodedUnicastAddress(stream, pimRegisterStop, pimRegisterStop->getSourceAddressForUpdate());
            length += ENCODED_UNICODE_ADDRESS_LENGTH;
            pimRegisterStop->setChunkLength(length);
            return pimRegisterStop;
        }
        case GraftAck:
        case Graft:
        case JoinPrune: {
            auto pimJoinPrune = makeShared<PimJoinPrune>();
            if (pimPacket->getType() == Graft || pimPacket->getType() == GraftAck)
                pimJoinPrune = makeShared<PimGraft>();
            pimJoinPrune->setType(pimPacket->getType());
            pimJoinPrune->setVersion(pimPacket->getVersion());
            pimJoinPrune->setReserved(pimPacket->getReserved());
            pimJoinPrune->setCrc(pimPacket->getCrc());
            pimJoinPrune->setCrcMode(pimPacket->getCrcMode());
            deserializeEncodedUnicastAddress(stream, pimJoinPrune, pimJoinPrune->getUpstreamNeighborAddressForUpdate());
            length += ENCODED_UNICODE_ADDRESS_LENGTH;
            pimJoinPrune->setReserved2(stream.readByte()); // Reserved
            pimJoinPrune->setJoinPruneGroupsArraySize(stream.readByte());
            pimJoinPrune->setHoldTime(stream.readUint16Be());
            length += B(4);
            if ((pimPacket->getType() == Graft || pimPacket->getType() == GraftAck) && pimJoinPrune->getHoldTime() != 0)
                pimJoinPrune->markIncorrect();
            for (size_t i = 0; i < pimJoinPrune->getJoinPruneGroupsArraySize(); ++i) {
                deserializeEncodedGroupAddress(stream, pimJoinPrune, pimJoinPrune->getJoinPruneGroupsForUpdate(i).getGroupAddressForUpdate());
                length += ENCODED_GROUP_ADDRESS_LENGTH;
                auto& joinPruneGroup = pimJoinPrune->getJoinPruneGroupsForUpdate(i);
                joinPruneGroup.setJoinedSourceAddressArraySize(stream.readUint16Be());
                joinPruneGroup.setPrunedSourceAddressArraySize(stream.readUint16Be());
                length += B(4);
                for (size_t k = 0; k < joinPruneGroup.getJoinedSourceAddressArraySize(); ++k) {
                    deserializeEncodedSourceAddress(stream, pimJoinPrune, joinPruneGroup.getJoinedSourceAddressForUpdate(k));
                    length += ENCODED_SOURCE_ADDRESS_LENGTH;
                }
                for (size_t k = 0; k < pimJoinPrune->getJoinPruneGroups(i).getPrunedSourceAddressArraySize(); ++k) {
                    deserializeEncodedSourceAddress(stream, pimJoinPrune, joinPruneGroup.getPrunedSourceAddressForUpdate(k));
                    length += ENCODED_SOURCE_ADDRESS_LENGTH;
                }
            }
            pimJoinPrune->setChunkLength(length);
            return pimJoinPrune;
        }
        case Assert: {
            auto pimAssert = makeShared<PimAssert>();
            pimAssert->setType(pimPacket->getType());
            pimAssert->setVersion(pimPacket->getVersion());
            pimAssert->setReserved(pimPacket->getReserved());
            pimAssert->setCrc(pimPacket->getCrc());
            pimAssert->setCrcMode(pimPacket->getCrcMode());
            deserializeEncodedGroupAddress(stream, pimAssert, pimAssert->getGroupAddressForUpdate());
            length += ENCODED_GROUP_ADDRESS_LENGTH;
            deserializeEncodedUnicastAddress(stream, pimAssert, pimAssert->getSourceAddressForUpdate());
            length += ENCODED_UNICODE_ADDRESS_LENGTH;
            pimAssert->setR(stream.readBit());
            pimAssert->setMetricPreference(stream.readNBitsToUint64Be(31));
            pimAssert->setMetric(stream.readUint32Be());
            length += B(8);
            return pimAssert;
        }
        case StateRefresh: {
            auto pimStateRefresh = makeShared<PimStateRefresh>();
            pimStateRefresh->setType(pimPacket->getType());
            pimStateRefresh->setVersion(pimPacket->getVersion());
            pimStateRefresh->setReserved(pimPacket->getReserved());
            pimStateRefresh->setCrc(pimPacket->getCrc());
            pimStateRefresh->setCrcMode(pimPacket->getCrcMode());
            deserializeEncodedGroupAddress(stream, pimStateRefresh, pimStateRefresh->getGroupAddressForUpdate());
            length += ENCODED_GROUP_ADDRESS_LENGTH;
            deserializeEncodedUnicastAddress(stream, pimStateRefresh, pimStateRefresh->getSourceAddressForUpdate());
            length += ENCODED_UNICODE_ADDRESS_LENGTH;
            deserializeEncodedUnicastAddress(stream, pimStateRefresh, pimStateRefresh->getOriginatorAddressForUpdate());
            length += ENCODED_UNICODE_ADDRESS_LENGTH;
            stream.readBit(); // R: The Rendezvous Point Tree bit.  Set to 0 for PIM-DM.  Ignored upon receipt.
            pimStateRefresh->setMetricPreference(stream.readNBitsToUint64Be(31));
            pimStateRefresh->setMetric(stream.readUint32Be());
            pimStateRefresh->setMaskLen(stream.readByte());
            pimStateRefresh->setTtl(stream.readByte());
            pimStateRefresh->setP(stream.readBit());
            pimStateRefresh->setN(stream.readBit());
            pimStateRefresh->setO(stream.readBit());
            pimStateRefresh->setReserved2(stream.readNBitsToUint64Be(5)); // Reserved
            pimStateRefresh->setInterval(stream.readByte());
            length += B(12);
            return pimStateRefresh;
        }
        default:
            throw cRuntimeError("Cannot serialize PIM packet: type %d not supported.", pimPacket->getType());
    }
}

} // namespace inet

