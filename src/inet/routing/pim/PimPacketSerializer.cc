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
#include "inet/routing/pim/PimPacket_m.h"
#include "inet/routing/pim/PimPacketSerializer.h"

namespace inet {

Register_Serializer(PimPacket, PimPacketSerializer);

Register_Serializer(PimHello, PimPacketSerializer);
Register_Serializer(PimJoinPrune, PimPacketSerializer);
Register_Serializer(PimAssert, PimPacketSerializer);
Register_Serializer(PimGraft, PimPacketSerializer);
Register_Serializer(PimStateRefresh, PimPacketSerializer);
Register_Serializer(PimRegister, PimPacketSerializer);
Register_Serializer(PimRegisterStop, PimPacketSerializer);

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
                        const HoldtimeOption* holdtimeOption = static_cast<const HoldtimeOption*>(pimHello->getOptions(i));
                        stream.writeUint16Be(2);    // length
                        stream.writeUint16Be(holdtimeOption->getHoldTime());
                        break;
                    }
                    case LANPruneDelay: {
                        const LanPruneDelayOption* lanPruneDelayOption = static_cast<const LanPruneDelayOption*>(pimHello->getOptions(i));
                        stream.writeUint16Be(4);    // length
                        stream.writeBit(0); // FIXME: T bit missing
                        stream.writeNBitsOfUint64Be(lanPruneDelayOption->getPropagationDelay(), 15);
                        stream.writeUint16Be(lanPruneDelayOption->getOverrideInterval());
                        break;
                    }
                    case DRPriority: {
                        const DrPriorityOption* drPriorityOption = static_cast<const DrPriorityOption*>(pimHello->getOptions(i));
                        stream.writeUint16Be(4);    // length
                        stream.writeUint32Be(drPriorityOption->getPriority());
                        break;
                    }
                    case GenerationID: {
                        const GenerationIdOption* generationIdOption = static_cast<const GenerationIdOption*>(pimHello->getOptions(i));
                        stream.writeUint16Be(4);    // length
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
            // Encoded-Group Address:
            stream.writeByte(1);    // Address Family: '1' represents the IPv4 address family
            stream.writeByte(0);    // Encoding Type: '0' represents the native encoding of the Address Family.
            stream.writeBit(0); // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
            stream.writeBitRepeatedly(0, 6);    // Reserved
            stream.writeBit(0); // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
            stream.writeByte(32);   // Mask Length: 32 for IPv4 native encoding
            stream.writeIpv4Address(pimRegisterStop->getGroupAddress());
            // Encoded-Unicast Address
            stream.writeByte(1);    // Address Family: '1' represents the IPv4 address family
            stream.writeByte(0);    // Encoding Type: '0' represents the native encoding of the Address Family.
            stream.writeIpv4Address(pimRegisterStop->getSourceAddress());
            break;
        }
        case GraftAck:
        case Graft:
        case JoinPrune: {
            const auto& pimJoinPrune = staticPtrCast<const PimJoinPrune>(chunk);
            // Encoded-Unicast Address
            stream.writeByte(1);    // Address Family: '1' represents the IPv4 address family
            stream.writeByte(0);    // Encoding Type: '0' represents the native encoding of the Address Family.
            stream.writeIpv4Address(pimJoinPrune->getUpstreamNeighborAddress());
            stream.writeByte(0);
            stream.writeByte(pimJoinPrune->getJoinPruneGroupsArraySize());
            if (pimPacket->getType() == Graft || pimPacket->getType() == GraftAck)
                stream.writeUint16Be(0);
            else
                stream.writeUint16Be(pimJoinPrune->getHoldTime());
            for (size_t i = 0; i < pimJoinPrune->getJoinPruneGroupsArraySize(); ++i) {
                // Encoded-Group Address:
                stream.writeByte(1);    // Address Family: '1' represents the IPv4 address family
                stream.writeByte(0);    // Encoding Type: '0' represents the native encoding of the Address Family.
                stream.writeBit(0); // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
                stream.writeBitRepeatedly(0, 6);    // Reserved
                stream.writeBit(0); // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
                stream.writeByte(32);   // Mask Length: 32 for IPv4 native encoding
                auto& joinPruneGroups = pimJoinPrune->getJoinPruneGroups(i);
                stream.writeIpv4Address(joinPruneGroups.getGroupAddress());
                stream.writeUint16Be(joinPruneGroups.getJoinedSourceAddressArraySize());
                stream.writeUint16Be(joinPruneGroups.getPrunedSourceAddressArraySize());
                for (size_t k = 0; k < joinPruneGroups.getJoinedSourceAddressArraySize(); ++k) {
                    // Encoded-Source Address
                    stream.writeByte(1);    // Address Family: '1' represents the IPv4 address family
                    stream.writeByte(0);    // Encoding Type: '0' represents the native encoding of the Address Family.
                    stream.writeNBitsOfUint64Be(0, 5);  // Reserved
                    auto& joinSourceAddress = joinPruneGroups.getJoinedSourceAddress(k);
                    stream.writeBit(joinSourceAddress.S);
                    stream.writeBit(joinSourceAddress.W);
                    stream.writeBit(joinSourceAddress.R);
                    stream.writeByte(32);   // Mask Length: 32 for IPv4 native encoding
                    stream.writeIpv4Address(joinPruneGroups.getJoinedSourceAddress(k).IPaddress);
                }
                for (size_t k = 0; k < joinPruneGroups.getPrunedSourceAddressArraySize(); ++k) {
                    // Encoded-Source Address
                    stream.writeByte(1);    // Address Family: '1' represents the IPv4 address family
                    stream.writeByte(0);    // Encoding Type: '0' represents the native encoding of the Address Family.
                    stream.writeNBitsOfUint64Be(0, 5);  // Reserved
                    auto& prunedSourceAddress = joinPruneGroups.getPrunedSourceAddress(k);
                    stream.writeBit(prunedSourceAddress.S);
                    stream.writeBit(prunedSourceAddress.W);
                    stream.writeBit(prunedSourceAddress.R);
                    stream.writeByte(32);   // Mask Length: 32 for IPv4 native encoding
                    stream.writeIpv4Address(prunedSourceAddress.IPaddress);
                }
            }
            break;
        }
        case Assert: {
            const auto& pimAssert = staticPtrCast<const PimAssert>(chunk);
            // Encoded-Group Address:
            stream.writeByte(1);    // Address Family: '1' represents the IPv4 address family
            stream.writeByte(0);    // Encoding Type: '0' represents the native encoding of the Address Family.
            stream.writeBit(0); // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
            stream.writeBitRepeatedly(0, 6);    // Reserved
            stream.writeBit(0); // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
            stream.writeByte(32);   // Mask Length: 32 for IPv4 native encoding
            stream.writeIpv4Address(pimAssert->getGroupAddress());
            // Encoded-Unicast Address
            stream.writeByte(1);    // Address Family: '1' represents the IPv4 address family
            stream.writeByte(0);    // Encoding Type: '0' represents the native encoding of the Address Family.
            stream.writeIpv4Address(pimAssert->getSourceAddress()); // address
            stream.writeBit(pimAssert->getR());
            stream.writeNBitsOfUint64Be(pimAssert->getMetricPreference(), 31);
            stream.writeUint32Be(pimAssert->getMetric());
            break;
        }
        case StateRefresh: {
            const auto& pimStateRefresh = staticPtrCast<const PimStateRefresh>(chunk);
            // Encoded-Group Address:
            stream.writeByte(1);    // Address Family: '1' represents the IPv4 address family
            stream.writeByte(0);    // Encoding Type: '0' represents the native encoding of the Address Family.
            stream.writeBit(0); // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
            stream.writeBitRepeatedly(0, 6);    // Reserved
            stream.writeBit(0); // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
            stream.writeByte(32);   // Mask Length: 32 for IPv4 native encoding
            stream.writeIpv4Address(pimStateRefresh->getGroupAddress());
            // Encoded-Unicast Address
            stream.writeByte(1);    // Address Family: '1' represents the IPv4 address family
            stream.writeByte(0);    // Encoding Type: '0' represents the native encoding of the Address Family.
            stream.writeIpv4Address(pimStateRefresh->getSourceAddress());
            // Encoded-Unicast Address
            stream.writeByte(1);    // Address Family: '1' represents the IPv4 address family
            stream.writeByte(0);    // Encoding Type: '0' represents the native encoding of the Address Family.
            stream.writeIpv4Address(pimStateRefresh->getOriginatorAddress());
            stream.writeBit(0); // R: The Rendezvous Point Tree bit.  Set to 0 for PIM-DM.  Ignored upon receipt.
            stream.writeNBitsOfUint64Be(pimStateRefresh->getMetricPreference(), 31);
            stream.writeUint32Be(pimStateRefresh->getMetric());
            stream.writeByte(pimStateRefresh->getMaskLen());
            stream.writeByte(pimStateRefresh->getTtl());
            stream.writeBit(pimStateRefresh->getP());
            // N:  Prune Now flag.  This SHOULD be set to 1 by the State Refresh
            //     originator on every third State Refresh message and SHOULD be
            //     ignored upon receipt.
            stream.writeBit(0);
            // O:  Assert Override flag.  This SHOULD be set to 1 by upstream routers
            //     on a LAN if the Assert Timer (AT(S,G)) is not running and SHOULD be
            //     ignored upon receipt.
            stream.writeBit(0);
            stream.writeBitRepeatedly(0, 5);
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
    //pimPacket->setCrcMode(pimPacket->getCrc() == 0 ? CRC_DISABLED : CRC_COMPUTED);  // FIXME: crcMode not set through the crcMode parameter
    pimPacket->setCrcMode(CRC_MODE_UNDEFINED);  // FIXME: crcMode not set through the crcMode parameter
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
                        bool T = stream.readBit();   // T bit
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
            // Encoded-Group Address:
            uint8_t addressFamily = stream.readByte();  // Address Family: '1' represents the IPv4 address family
            uint8_t encodingType = stream.readByte();  // Encoding Type: '0' represents the native encoding of the Address Family.
            bool bidirectional = stream.readBit();   // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
            uint8_t reserved = stream.readNBitsToUint64Be(6); // Reserved
            bool adminScopeZone = stream.readBit();   // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
            uint8_t maskLength = stream.readByte();  // Mask Length: 32 for IPv4 native encoding
            if (addressFamily != 1 || encodingType != 0 || bidirectional || reserved != 0 || adminScopeZone || maskLength != 32)
                pimRegisterStop->markIncorrect();
            pimRegisterStop->setGroupAddress(stream.readIpv4Address());
            // Encoded-Unicast Address
            addressFamily = stream.readByte();  // Address Family: '1' represents the IPv4 address family
            encodingType = stream.readByte();  // Encoding Type: '0' represents the native encoding of the Address Family.
            if (addressFamily != 1 || encodingType != 0)
                pimRegisterStop->markIncorrect();
            pimRegisterStop->setSourceAddress(stream.readIpv4Address());
            length += (ENCODED_GROUP_ADDRESS_LENGTH + ENCODED_UNICODE_ADDRESS_LENGTH);
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
            // Encoded-Unicast Address
            uint8_t addressFamily = stream.readByte();  // Address Family: '1' represents the IPv4 address family
            uint8_t encodingType = stream.readByte();  // Encoding Type: '0' represents the native encoding of the Address Family.
            if (addressFamily != 1 || encodingType != 0)
                pimJoinPrune->markIncorrect();
            pimJoinPrune->setUpstreamNeighborAddress(stream.readIpv4Address());
            uint8_t reserved = stream.readByte();  // Reserved
            if (reserved != 0)
                pimJoinPrune->markIncorrect();
            pimJoinPrune->setJoinPruneGroupsArraySize(stream.readByte());
            pimJoinPrune->setHoldTime(stream.readUint16Be());
            length += (ENCODED_UNICODE_ADDRESS_LENGTH + B(4));
            if ((pimPacket->getType() == Graft || pimPacket->getType() == GraftAck) && pimJoinPrune->getHoldTime() != 0)
                pimJoinPrune->markIncorrect();
            for (size_t i = 0; i < pimJoinPrune->getJoinPruneGroupsArraySize(); ++i) {
                // Encoded-Group Address:
                addressFamily = stream.readByte();  // Address Family: '1' represents the IPv4 address family
                encodingType = stream.readByte();  // Encoding Type: '0' represents the native encoding of the Address Family.
                bool bidirectional = stream.readBit();   // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
                reserved = stream.readNBitsToUint64Be(6); // Reserved
                bool adminScopeZone = stream.readBit();   // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
                uint8_t maskLength = stream.readByte();  // Mask Length: 32 for IPv4 native encoding
                if (addressFamily != 1 || encodingType != 0 || bidirectional || reserved != 0 || adminScopeZone || maskLength != 32)
                    pimJoinPrune->markIncorrect();
                pimJoinPrune->getJoinPruneGroupsForUpdate(i).setGroupAddress(stream.readIpv4Address());
                length += ENCODED_GROUP_ADDRESS_LENGTH;
                auto& joinPruneGroup = pimJoinPrune->getJoinPruneGroupsForUpdate(i);
                joinPruneGroup.setJoinedSourceAddressArraySize(stream.readUint16Be());
                joinPruneGroup.setPrunedSourceAddressArraySize(stream.readUint16Be());
                length += B(4);
                for (size_t k = 0; k < joinPruneGroup.getJoinedSourceAddressArraySize(); ++k) {
                    // Encoded-Source Address
                    addressFamily = stream.readByte();  // Address Family: '1' represents the IPv4 address family
                    encodingType = stream.readByte();  // Encoding Type: '0' represents the native encoding of the Address Family.
                    reserved = stream.readNBitsToUint64Be(5);  // Reserved
                    auto& joinedSourceAddress = joinPruneGroup.getJoinedSourceAddressForUpdate(k);
                    joinedSourceAddress.S = stream.readBit();
                    joinedSourceAddress.W = stream.readBit();
                    joinedSourceAddress.R = stream.readBit();
                    maskLength = stream.readByte();  // Mask Length: 32 for IPv4 native encoding
                    if (addressFamily != 1 || encodingType != 0 || reserved != 0 || maskLength != 32)
                        pimJoinPrune->markIncorrect();
                    joinedSourceAddress.IPaddress = stream.readIpv4Address();
                    length += ENCODED_SOURCE_ADDRESS_LENGTH;
                }
                for (size_t k = 0; k < pimJoinPrune->getJoinPruneGroups(i).getPrunedSourceAddressArraySize(); ++k) {
                    // Encoded-Source Address
                    addressFamily = stream.readByte();  // Address Family: '1' represents the IPv4 address family
                    encodingType = stream.readByte();  // Encoding Type: '0' represents the native encoding of the Address Family.
                    reserved = stream.readNBitsToUint64Be(5);  // Reserved
                    auto& prunedSourceAddress = joinPruneGroup.getPrunedSourceAddressForUpdate(k);
                    prunedSourceAddress.S = stream.readBit();
                    prunedSourceAddress.W = stream.readBit();
                    prunedSourceAddress.R = stream.readBit();
                    maskLength = stream.readByte();  // Mask Length: 32 for IPv4 native encoding
                    if (addressFamily != 1 || encodingType != 0 || reserved != 0 || maskLength != 32)
                        pimJoinPrune->markIncorrect();
                    prunedSourceAddress.IPaddress = stream.readIpv4Address();
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
            // Encoded-Group Address:
            uint8_t addressFamily = stream.readByte();  // Address Family: '1' represents the IPv4 address family
            uint8_t encodingType = stream.readByte();  // Encoding Type: '0' represents the native encoding of the Address Family.
            bool bidirectional = stream.readBit();   // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
            uint8_t reserved = stream.readNBitsToUint64Be(6); // Reserved
            bool adminScopeZone = stream.readBit();   // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
            uint8_t maskLength = stream.readByte();  // Mask Length: 32 for IPv4 native encoding
            if (addressFamily != 1 || encodingType != 0 || bidirectional || reserved != 0 || adminScopeZone || maskLength != 32)
                pimAssert->markIncorrect();
            pimAssert->setGroupAddress(stream.readIpv4Address());
            // Encoded-Unicast Address
            addressFamily = stream.readByte();  // Address Family: '1' represents the IPv4 address family
            encodingType = stream.readByte();  // Encoding Type: '0' represents the native encoding of the Address Family.
            if (addressFamily != 1 || encodingType != 0)
                pimAssert->markIncorrect();
            pimAssert->setSourceAddress(stream.readIpv4Address());
            pimAssert->setR(stream.readBit());
            pimAssert->setMetricPreference(stream.readNBitsToUint64Be(31));
            pimAssert->setMetric(stream.readUint32Be());
            length += (ENCODED_GROUP_ADDRESS_LENGTH + ENCODED_UNICODE_ADDRESS_LENGTH + B(8));
            return pimAssert;
        }
        case StateRefresh: {
            auto pimStateRefresh = makeShared<PimStateRefresh>();
            pimStateRefresh->setType(pimPacket->getType());
            pimStateRefresh->setVersion(pimPacket->getVersion());
            pimStateRefresh->setReserved(pimPacket->getReserved());
            pimStateRefresh->setCrc(pimPacket->getCrc());
            pimStateRefresh->setCrcMode(pimPacket->getCrcMode());
            // Encoded-Group Address:
            uint8_t addressFamily = stream.readByte();  // Address Family: '1' represents the IPv4 address family
            uint8_t encodingType = stream.readByte();  // Encoding Type: '0' represents the native encoding of the Address Family.
            bool bidirectional = stream.readBit();   // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
            uint8_t reserved = stream.readNBitsToUint64Be(6); // Reserved
            bool adminScopeZone = stream.readBit();   // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
            uint8_t maskLength = stream.readByte();  // Mask Length: 32 for IPv4 native encoding
            if (addressFamily != 1 || encodingType != 0 || bidirectional || reserved != 0 || adminScopeZone || maskLength != 32)
                pimStateRefresh->markIncorrect();
            pimStateRefresh->setGroupAddress(stream.readIpv4Address());
            // Encoded-Unicast Address
            addressFamily = stream.readByte();  // Address Family: '1' represents the IPv4 address family
            encodingType = stream.readByte();  // Encoding Type: '0' represents the native encoding of the Address Family.
            if (addressFamily != 1 || encodingType != 0)
                pimStateRefresh->markIncorrect();
            pimStateRefresh->setSourceAddress(stream.readIpv4Address());
            // Encoded-Unicast Address
            addressFamily = stream.readByte();  // Address Family: '1' represents the IPv4 address family
            encodingType = stream.readByte();  // Encoding Type: '0' represents the native encoding of the Address Family.
            if (addressFamily != 1 || encodingType != 0)
                pimStateRefresh->markIncorrect();
            pimStateRefresh->setOriginatorAddress(stream.readIpv4Address());
            stream.readBit();   // R: The Rendezvous Point Tree bit.  Set to 0 for PIM-DM.  Ignored upon receipt.
            pimStateRefresh->setMetricPreference(stream.readNBitsToUint64Be(31));
            pimStateRefresh->setMetric(stream.readUint32Be());
            pimStateRefresh->setMaskLen(stream.readByte());
            pimStateRefresh->setTtl(stream.readByte());
            pimStateRefresh->setP(stream.readBit());
            // N:  Prune Now flag.  This SHOULD be set to 1 by the State Refresh
            //     originator on every third State Refresh message and SHOULD be
            //     ignored upon receipt.
            bool pruneNowFlag = stream.readBit();
            // O:  Assert Override flag.  This SHOULD be set to 1 by upstream routers
            //     on a LAN if the Assert Timer (AT(S,G)) is not running and SHOULD be
            //     ignored upon receipt.
            bool assertOverrideFlag = stream.readBit();
            reserved = stream.readNBitsToUint64Be(5); // Reserved
            if (pruneNowFlag || assertOverrideFlag || reserved != 0)
                pimStateRefresh->markIncorrect();
            pimStateRefresh->setInterval(stream.readByte());
            length += (ENCODED_GROUP_ADDRESS_LENGTH + ENCODED_UNICODE_ADDRESS_LENGTH + ENCODED_UNICODE_ADDRESS_LENGTH + B(12));
            return pimStateRefresh;
        }
        default:
            throw cRuntimeError("Cannot serialize PIM packet: type %d not supported.", pimPacket->getType());
    }
}

} // namespace inet


















