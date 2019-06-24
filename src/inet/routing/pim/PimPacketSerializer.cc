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
    stream.writeByte(0);
    // FIXME: checksum??
    stream.writeUint16Be(0);
    switch (pimPacket->getType()) {
        case Hello: {
            const auto& pimHello = staticPtrCast<const PimHello>(chunk);
            for (size_t i = 0; i < pimHello->getOptionsArraySize(); ++i) {
                stream.writeUint16Be(pimHello->getOptions(i)->getType());
                switch (pimHello->getOptions(i)->getType()) {
                    case Holdtime: {
                        const HoldtimeOption* holdtimeOption = static_cast<const HoldtimeOption*>(pimHello->getOptions(i));
                        // length = 2
                        stream.writeUint16Be(2);
                        stream.writeUint16Be(holdtimeOption->getHoldTime());
                        break;
                    }
                    case LANPruneDelay: {
                        const LanPruneDelayOption* lanPruneDelayOption = static_cast<const LanPruneDelayOption*>(pimHello->getOptions(i));
                        // length = 4
                        stream.writeUint16Be(4);
                        // FIXME: T bit missing
                        stream.writeBit(0);
                        stream.writeNBitsOfUint64Be(lanPruneDelayOption->getPropagationDelay(), 15);
                        stream.writeUint16Be(lanPruneDelayOption->getOverrideInterval());
                        break;
                    }
                    case DRPriority: {
                        const DrPriorityOption* drPriorityOption = static_cast<const DrPriorityOption*>(pimHello->getOptions(i));
                        // length = 4
                        stream.writeUint16Be(4);
                        stream.writeUint32Be(drPriorityOption->getPriority());
                        break;
                    }
                    case GenerationID: {
                        const GenerationIdOption* generationIdOption = static_cast<const GenerationIdOption*>(pimHello->getOptions(i));
                        // length = 4
                        stream.writeUint16Be(4);
                        stream.writeUint32Be(generationIdOption->getGenerationID());
                        break;
                    }
                    default:
                        throw cRuntimeError("Cannot serialize Hello options: type %d not supported.", pimHello->getOptions(i)->getType());
                }
            }
            // FIXME: how to detect the end of the options field in the deserializer part?
            stream.writeUint32Be(10);
            break;
        }
        case Register: {
            const auto& pimRegister = staticPtrCast<const PimRegister>(chunk);
            stream.writeBit(pimRegister->getB());
            stream.writeBit(pimRegister->getN());
            stream.writeNBitsOfUint64Be(0, 30);
            // FIXME: Multicast data packet: ?
            stream.writeUint64Be(0);
            break;
        }
        case RegisterStop: {
            const auto& pimRegisterStop = staticPtrCast<const PimRegisterStop>(chunk);
            // Encoded-Group Address:
            // Address Family: IPv4
            stream.writeByte(1);
            // FIXME: Encoding Type: ?
            // The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.writeByte(0);
            // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
            stream.writeBit(0);
            // reserved:
            stream.writeBitRepeatedly(0, 6);
            // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
            stream.writeBit(0);
            // FIXME: Mask Length: 32 for IPv4 native encoding
            stream.writeByte(32);
            stream.writeIpv4Address(pimRegisterStop->getGroupAddress());
            // Encoded-Unicast Address
            // Address Family: IPv4
            stream.writeByte(1);
            // FIXME: Encoding Type: ?
            // The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.writeByte(0);
            stream.writeIpv4Address(pimRegisterStop->getSourceAddress());
            break;
        }
        case Graft:
        case JoinPrune: {
            const auto& pimJoinPrune = staticPtrCast<const PimJoinPrune>(chunk);
            // Encoded-Unicast Address
            // Address Family: IPv4
            stream.writeByte(1);
            // FIXME: Encoding Type: ?
            // The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.writeByte(0);
            stream.writeIpv4Address(pimJoinPrune->getUpstreamNeighborAddress());
            stream.writeByte(0);
            stream.writeByte(pimJoinPrune->getJoinPruneGroupsArraySize());
            if (pimPacket->getType() == Graft)
                stream.writeUint16Be(0);
            else
                stream.writeUint16Be(pimJoinPrune->getHoldTime());
            for (size_t i = 0; i < pimJoinPrune->getJoinPruneGroupsArraySize(); ++i) {
                // Encoded-Group Address:
                // Address Family: IPv4
                stream.writeByte(1);
                // FIXME: Encoding Type: ?
                // The value '0' is reserved for this field and represents the native encoding of the Address Family.
                stream.writeByte(0);
                // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
                stream.writeBit(0);
                // reserved:
                stream.writeBitRepeatedly(0, 6);
                // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
                stream.writeBit(0);
                // FIXME: Mask Length: 32 for IPv4 native encoding
                stream.writeByte(32);
                stream.writeIpv4Address(pimJoinPrune->getJoinPruneGroups(i).getGroupAddress());
                stream.writeUint32Be(pimJoinPrune->getJoinPruneGroups(i).getJoinedSourceAddressArraySize());
                stream.writeUint32Be(pimJoinPrune->getJoinPruneGroups(i).getPrunedSourceAddressArraySize());
                for (size_t k = 0; k < pimJoinPrune->getJoinPruneGroups(i).getJoinedSourceAddressArraySize(); ++k) {
                    // Encoded-Source Address
                    // Address Family: IPv4
                    stream.writeByte(1);
                    // FIXME: Encoding Type: ?
                    // The value '0' is reserved for this field and represents the native encoding of the Address Family.
                    stream.writeByte(0);
                    // Reserved:
                    stream.writeNBitsOfUint64Be(0, 5);
                    stream.writeBit(pimJoinPrune->getJoinPruneGroups(i).getJoinedSourceAddress(k).S);
                    stream.writeBit(pimJoinPrune->getJoinPruneGroups(i).getJoinedSourceAddress(k).W);
                    stream.writeBit(pimJoinPrune->getJoinPruneGroups(i).getJoinedSourceAddress(k).R);
                    // FIXME: Mask Length: 32 for IPv4 native encoding
                    stream.writeByte(32);
                    stream.writeIpv4Address(pimJoinPrune->getJoinPruneGroups(i).getJoinedSourceAddress(k).IPaddress);
                }
                for (size_t k = 0; k < pimJoinPrune->getJoinPruneGroups(i).getPrunedSourceAddressArraySize(); ++k) {
                    // Encoded-Source Address
                    // Address Family: IPv4
                    stream.writeByte(1);
                    // FIXME: Encoding Type: ?
                    // The value '0' is reserved for this field and represents the native encoding of the Address Family.
                    stream.writeByte(0);
                    // Reserved:
                    stream.writeNBitsOfUint64Be(0, 5);
                    stream.writeBit(pimJoinPrune->getJoinPruneGroups(i).getPrunedSourceAddress(k).S);
                    stream.writeBit(pimJoinPrune->getJoinPruneGroups(i).getPrunedSourceAddress(k).W);
                    stream.writeBit(pimJoinPrune->getJoinPruneGroups(i).getPrunedSourceAddress(k).R);
                    // FIXME: Mask Length: 32 for IPv4 native encoding
                    stream.writeByte(32);
                    stream.writeIpv4Address(pimJoinPrune->getJoinPruneGroups(i).getPrunedSourceAddress(k).IPaddress);
                }
            }
            break;
        }
        case Assert: {
            const auto& pimAssert = staticPtrCast<const PimAssert>(chunk);
            // Encoded-Group Address:
            // Address Family: IPv4
            stream.writeByte(1);
            // FIXME: Encoding Type: ?
            // The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.writeByte(0);
            // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
            stream.writeBit(0);
            // reserved:
            stream.writeBitRepeatedly(0, 6);
            // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
            stream.writeBit(0);
            // FIXME: Mask Length: 32 for IPv4 native encoding
            stream.writeByte(32);
            stream.writeIpv4Address(pimAssert->getGroupAddress());
            // Encoded-Unicast Address
            // Address Family: IPv4
            stream.writeByte(1);
            // FIXME: Encoding Type: ?
            // The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.writeByte(0);
            // address:
            stream.writeIpv4Address(pimAssert->getSourceAddress());
            stream.writeBit(pimAssert->getR());
            stream.writeNBitsOfUint64Be(pimAssert->getMetricPreference(), 31);
            stream.writeUint32Be(pimAssert->getMetric());
            break;
        }
        case StateRefresh: {
            const auto& pimStateRefresh = staticPtrCast<const PimStateRefresh>(chunk);
            // Encoded-Group Address:
            // Address Family: IPv4
            stream.writeByte(1);
            // FIXME: Encoding Type: ?
            // The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.writeByte(0);
            // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
            stream.writeBit(0);
            // reserved:
            stream.writeBitRepeatedly(0, 6);
            // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
            stream.writeBit(0);
            // FIXME: Mask Length: 32 for IPv4 native encoding
            stream.writeByte(32);
            stream.writeIpv4Address(pimStateRefresh->getGroupAddress());
            // Encoded-Unicast Address
            // Address Family: IPv4
            stream.writeByte(1);
            // FIXME: Encoding Type: ?
            // The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.writeByte(0);
            // address:
            stream.writeIpv4Address(pimStateRefresh->getSourceAddress());
            // Encoded-Unicast Address
            // Address Family: IPv4
            stream.writeByte(1);
            // FIXME: Encoding Type: ?
            // The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.writeByte(0);
            // address:
            stream.writeIpv4Address(pimStateRefresh->getOriginatorAddress());
            // R: The Rendezvous Point Tree bit.  Set to 0 for PIM-DM.  Ignored upon receipt.
            stream.writeBit(0);
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
    stream.readByte();
    // FIXME: checksum??
    stream.readUint16Be();
    switch (pimPacket->getType()) {
        case Hello: {
            auto pimHello = makeShared<PimHello>();
            pimHello->setType(pimPacket->getType());
            pimHello->setVersion(pimPacket->getVersion());
            PimHelloOptionType type = static_cast<PimHelloOptionType>(stream.readUint32Be());
            while (type != static_cast<PimHelloOptionType>(10)) {
                switch (type) {
                    case Holdtime: {
                        stream.readUint16Be();
                        pimHello->setOptionsArraySize(pimHello->getOptionsArraySize() + 1);
                        //HoldtimeOption* holdtimeOption = static_cast<HoldtimeOption*>(pimHello->getOptionsForUpdate(pimHello->getOptionsArraySize() - 1));
                        //holdtimeOption->setHoldTime(stream.readUint32Be());
                        //pimHello->setOptions(pimHello->getOptionsArraySize() - 1, holdtimeOption);
                        static_cast<HoldtimeOption*>(pimHello->getOptionsForUpdate(pimHello->getOptionsArraySize() - 1))->setHoldTime(stream.readUint32Be());

                        type = static_cast<PimHelloOptionType>(stream.readUint32Be());
                        break;
                    }
                    case LANPruneDelay: {
                        stream.readUint16Be();
                        stream.readBit();
                        pimHello->setOptionsArraySize(pimHello->getOptionsArraySize() + 1);
                        LanPruneDelayOption* lanPruneDelayOption = static_cast<LanPruneDelayOption*>(pimHello->getOptionsForUpdate(pimHello->getOptionsArraySize() - 1));
                        lanPruneDelayOption->setPropagationDelay(stream.readNBitsToUint64Be(15));
                        lanPruneDelayOption->setOverrideInterval(stream.readUint16Be());
                        pimHello->setOptions(pimHello->getOptionsArraySize() - 1, lanPruneDelayOption);

                        type = static_cast<PimHelloOptionType>(stream.readUint32Be());
                        break;
                    }
                    case DRPriority: {
                        stream.readUint16Be();
                        pimHello->setOptionsArraySize(pimHello->getOptionsArraySize() + 1);
                        //DrPriorityOption* drPriorityOption = static_cast<DrPriorityOption*>(pimHello->getOptionsForUpdate(pimHello->getOptionsArraySize() - 1));
                        //drPriorityOption->setPriority(stream.readUint32Be());
                        //pimHello->setOptions(pimHello->getOptionsArraySize() - 1, drPriorityOption);
                        static_cast<DrPriorityOption*>(pimHello->getOptionsForUpdate(pimHello->getOptionsArraySize() - 1))->setPriority(stream.readUint32Be());

                        type = static_cast<PimHelloOptionType>(stream.readUint32Be());
                        break;
                    }
                    case GenerationID: {
                        stream.readUint16Be();
                        pimHello->setOptionsArraySize(pimHello->getOptionsArraySize() + 1);
                        //GenerationIdOption* generationIdOption = static_cast<GenerationIdOption*>(pimHello->getOptionsForUpdate(pimHello->getOptionsArraySize() - 1));
                        //generationIdOption->setGenerationID(stream.readUint32Be());
                        //pimHello->setOptions(pimHello->getOptionsArraySize() - 1, generationIdOption);
                        static_cast<GenerationIdOption*>(pimHello->getOptionsForUpdate(pimHello->getOptionsArraySize() - 1))->setGenerationID(stream.readUint32Be());

                        type = static_cast<PimHelloOptionType>(stream.readUint32Be());
                        break;
                    }
                    case StateRefreshCapable: {
                        throw cRuntimeError("StateRefreshCapable is not implemented yet.");
                        break;
                    }
                    case AddressList: {
                        throw cRuntimeError("AddressList is not implemented yet.");
                        break;
                    }
                    default:
                        throw cRuntimeError("Cannot deserialize Hello options: type %d not supported.", type);
                }
            }
            return pimHello;
        }
        case Register: {
            auto pimRegister = makeShared<PimRegister>();
            pimRegister->setType(pimPacket->getType());
            pimRegister->setVersion(pimPacket->getVersion());
            pimRegister->setB(stream.readBit());
            pimRegister->setN(stream.readBit());
            stream.readNBitsToUint64Be(30);
            // FIXME: Multicast data packet: ?
            stream.readUint64Be();
            return pimRegister;
        }
        case RegisterStop: {
            auto pimRegisterStop = makeShared<PimRegisterStop>();
            pimRegisterStop->setType(pimPacket->getType());
            pimRegisterStop->setVersion(pimPacket->getVersion());
            // Encoded-Group Address:
            // Address Family: IPv4
            stream.readByte();
            // FIXME: Encoding Type: ?
            // The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.readByte();
            // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
            stream.readBit();
            // reserved:
            stream.readBitRepeatedly(0, 6);
            // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
            stream.readBit();
            // FIXME: Mask Length: 32 for IPv4 native encoding
            stream.readByte();
            pimRegisterStop->setGroupAddress(stream.readIpv4Address());
            // Encoded-Unicast Address
            // Address Family: IPv4
            stream.readByte();
            // FIXME: Encoding Type: ?
            // The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.readByte();
            pimRegisterStop->setSourceAddress(stream.readIpv4Address());
            return pimRegisterStop;
        }
        case Graft:
        case JoinPrune: {
            auto pimJoinPrune = makeShared<PimJoinPrune>();
            pimJoinPrune->setType(pimPacket->getType());
            pimJoinPrune->setVersion(pimPacket->getVersion());
            // Encoded-Unicast Address
            // Address Family: IPv4
            stream.readByte();
            // FIXME: Encoding Type: ?
            // The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.readByte();
            pimJoinPrune->setUpstreamNeighborAddress(stream.readIpv4Address());
            stream.readByte();
            pimJoinPrune->setJoinPruneGroupsArraySize(stream.readByte());
            pimJoinPrune->setHoldTime(stream.readUint16Be());
            if (pimPacket->getType() == Graft && pimJoinPrune->getHoldTime() != 0)
                pimJoinPrune->markIncorrect();
            for (size_t i = 0; i < pimJoinPrune->getJoinPruneGroupsArraySize(); ++i) {
                // Encoded-Group Address:
                // Address Family: IPv4
                stream.readByte();
                // FIXME: Encoding Type: ?
                // The value '0' is reserved for this field and represents the native encoding of the Address Family.
                stream.readByte();
                // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
                stream.readBit();
                // reserved:
                stream.readBitRepeatedly(0, 6);
                // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
                stream.readBit();
                // FIXME: Mask Length: 32 for IPv4 native encoding
                stream.readByte();
                pimJoinPrune->getJoinPruneGroupsForUpdate(i).setGroupAddress(stream.readIpv4Address());
                pimJoinPrune->getJoinPruneGroupsForUpdate(i).setJoinedSourceAddressArraySize(stream.readUint32Be());
                pimJoinPrune->getJoinPruneGroupsForUpdate(i).setPrunedSourceAddressArraySize(stream.readUint32Be());
                for (size_t k = 0; k < pimJoinPrune->getJoinPruneGroups(i).getJoinedSourceAddressArraySize(); ++k) {
                    // Encoded-Source Address
                    // Address Family: IPv4
                    stream.readByte();
                    // FIXME: Encoding Type: ?
                    // The value '0' is reserved for this field and represents the native encoding of the Address Family.
                    stream.readByte();
                    // Reserved:
                    stream.readNBitsToUint64Be(5);
                    EncodedAddress encodedAddress;

                    pimJoinPrune->getJoinPruneGroupsForUpdate(i).getJoinedSourceAddressForUpdate(k).S = stream.readBit();
                    pimJoinPrune->getJoinPruneGroupsForUpdate(i).getJoinedSourceAddressForUpdate(k).W = stream.readBit();
                    pimJoinPrune->getJoinPruneGroupsForUpdate(i).getJoinedSourceAddressForUpdate(k).R = stream.readBit();
                    // FIXME: Mask Length: 32 for IPv4 native encoding
                    stream.readByte();
                    pimJoinPrune->getJoinPruneGroupsForUpdate(i).getJoinedSourceAddressForUpdate(k).IPaddress = stream.readIpv4Address();
                }
                for (size_t k = 0; k < pimJoinPrune->getJoinPruneGroups(i).getPrunedSourceAddressArraySize(); ++k) {
                    // Encoded-Source Address
                    // Address Family: IPv4
                    stream.readByte();
                    // FIXME: Encoding Type: ?
                    // The value '0' is reserved for this field and represents the native encoding of the Address Family.
                    stream.readByte();
                    // Reserved:
                    stream.readNBitsToUint64Be(5);
                    pimJoinPrune->getJoinPruneGroupsForUpdate(i).getPrunedSourceAddressForUpdate(k).S = stream.readBit();
                    pimJoinPrune->getJoinPruneGroupsForUpdate(i).getPrunedSourceAddressForUpdate(k).W = stream.readBit();
                    pimJoinPrune->getJoinPruneGroupsForUpdate(i).getPrunedSourceAddressForUpdate(k).R = stream.readBit();
                    // FIXME: Mask Length: 32 for IPv4 native encoding
                    stream.readByte();
                    pimJoinPrune->getJoinPruneGroupsForUpdate(i).getPrunedSourceAddressForUpdate(k).IPaddress = stream.readIpv4Address();
                }
            }
            return pimJoinPrune;
        }
        case Assert: {
            auto pimAssert = makeShared<PimAssert>();
            pimAssert->setType(pimPacket->getType());
            pimAssert->setVersion(pimPacket->getVersion());
            // Encoded-Group Address:
            // Address Family: IPv4
            stream.readByte();
            // FIXME: Encoding Type: ?
            // The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.readByte();
            // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
            stream.readBit();
            // reserved:
            stream.readBitRepeatedly(0, 6);
            // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
            stream.readBit();
            // FIXME: Mask Length: 32 for IPv4 native encoding
            stream.readByte();
            pimAssert->setGroupAddress(stream.readIpv4Address());
            // Encoded-Unicast Address
            // Address Family: IPv4
            stream.readByte();
            // FIXME: Encoding Type: ?
            // The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.readByte();
            // address:
            pimAssert->setSourceAddress(stream.readIpv4Address());
            pimAssert->setR(stream.readBit());
            pimAssert->setMetricPreference(stream.readNBitsToUint64Be(31));
            pimAssert->setMetric(stream.readUint32Be());
            return pimAssert;
        }
        case StateRefresh: {
            auto pimStateRefresh = makeShared<PimStateRefresh>();
            pimStateRefresh->setType(pimPacket->getType());
            pimStateRefresh->setVersion(pimPacket->getVersion());
            // Encoded-Group Address:
            // Address Family: IPv4
            stream.readByte();
            // FIXME: Encoding Type: ?
            // The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.readByte();
            // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
            stream.readBit();
            // reserved:
            stream.readBitRepeatedly(0, 6);
            // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
            stream.readBit();
            // FIXME: Mask Length: 32 for IPv4 native encoding
            stream.readByte();
            pimStateRefresh->setGroupAddress(stream.readIpv4Address());
            // Encoded-Unicast Address
            // Address Family: IPv4
            stream.readByte();
            // FIXME: Encoding Type: ?
            // The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.readByte();
            // address:
            pimStateRefresh->setSourceAddress(stream.readIpv4Address());
            // Encoded-Unicast Address
            // Address Family: IPv4
            stream.readByte();
            // FIXME: Encoding Type: ?
            // The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.readByte();
            // address:
            pimStateRefresh->setOriginatorAddress(stream.readIpv4Address());
            // R: The Rendezvous Point Tree bit.  Set to 0 for PIM-DM.  Ignored upon receipt.
            stream.readBit();
            pimStateRefresh->setMetricPreference(stream.readNBitsToUint64Be(31));
            pimStateRefresh->setMetric(stream.readUint32Be());
            pimStateRefresh->setMaskLen(stream.readByte());
            pimStateRefresh->setTtl(stream.readByte());
            pimStateRefresh->setP(stream.readBit());
            // N:  Prune Now flag.  This SHOULD be set to 1 by the State Refresh
            //     originator on every third State Refresh message and SHOULD be
            //     ignored upon receipt.
            stream.readBit();
            // O:  Assert Override flag.  This SHOULD be set to 1 by upstream routers
            //     on a LAN if the Assert Timer (AT(S,G)) is not running and SHOULD be
            //     ignored upon receipt.
            stream.readBit();
            stream.readBitRepeatedly(0, 5);
            pimStateRefresh->setInterval(stream.readByte());
            return pimStateRefresh;
        }
        default:
            throw cRuntimeError("Cannot serialize PIM packet: type %d not supported.", pimPacket->getType());
    }
}

} // namespace inet


















