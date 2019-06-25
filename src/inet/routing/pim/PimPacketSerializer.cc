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

    b start = stream.getLength();

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
                        // FIXME: T bit missing
                        stream.writeBit(0);
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
            // FIXME: how to detect the end of the options field in the deserializer part?
            stream.writeUint16Be(10);

            std::cout << "PimHello: length of the stream: " << stream.getLength() - start << endl;
            std::cout << "PimHello: chunkLength: " << pimHello->getChunkLength() << endl;
            break;
        }
        case Register: {
            const auto& pimRegister = staticPtrCast<const PimRegister>(chunk);
            stream.writeBit(pimRegister->getB());
            stream.writeBit(pimRegister->getN());
            stream.writeNBitsOfUint64Be(0, 30);
            //stream.writeUint64Be(0);    // FIXME: Multicast data packet: ?

            std::cout << "PimRegister: length of the stream: " << stream.getLength() - start << endl;
            std::cout << "PimRegister: chunkLength: " << pimRegister->getChunkLength() << endl;
            break;
        }
        case RegisterStop: {
            const auto& pimRegisterStop = staticPtrCast<const PimRegisterStop>(chunk);
            // Encoded-Group Address:
            // Address Family: IPv4
            stream.writeByte(1);
            stream.writeByte(0);    // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.writeBit(0); // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
            stream.writeBitRepeatedly(0, 6);    // reserved
            stream.writeBit(0); // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
            stream.writeByte(32);   // FIXME: Mask Length: 32 for IPv4 native encoding
            stream.writeIpv4Address(pimRegisterStop->getGroupAddress());
            // Encoded-Unicast Address
            stream.writeByte(1);    // Address Family: IPv4
            stream.writeByte(0);    // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.writeIpv4Address(pimRegisterStop->getSourceAddress());

            std::cout << "PimRegisterStop: length of the stream: " << stream.getLength() - start << endl;
            std::cout << "PimRegisterStop: chunkLength: " << pimRegisterStop->getChunkLength() << endl;
            break;
        }
        case Graft:
        case JoinPrune: {
            const auto& pimJoinPrune = staticPtrCast<const PimJoinPrune>(chunk);
            // Encoded-Unicast Address
            stream.writeByte(1);    // Address Family: IPv4
            stream.writeByte(0);    // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.writeIpv4Address(pimJoinPrune->getUpstreamNeighborAddress());
            stream.writeByte(0);
            stream.writeByte(pimJoinPrune->getJoinPruneGroupsArraySize());
            if (pimPacket->getType() == Graft)
                stream.writeUint16Be(0);
            else
                stream.writeUint16Be(pimJoinPrune->getHoldTime());
            for (size_t i = 0; i < pimJoinPrune->getJoinPruneGroupsArraySize(); ++i) {
                // Encoded-Group Address:
                stream.writeByte(1);    // Address Family: IPv4
                stream.writeByte(0);    // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
                stream.writeBit(0); // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
                stream.writeBitRepeatedly(0, 6);    // reserved
                stream.writeBit(0); // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
                stream.writeByte(32);   // FIXME: Mask Length: 32 for IPv4 native encoding
                stream.writeIpv4Address(pimJoinPrune->getJoinPruneGroups(i).getGroupAddress());
                stream.writeUint16Be(pimJoinPrune->getJoinPruneGroups(i).getJoinedSourceAddressArraySize());
                stream.writeUint16Be(pimJoinPrune->getJoinPruneGroups(i).getPrunedSourceAddressArraySize());
                for (size_t k = 0; k < pimJoinPrune->getJoinPruneGroups(i).getJoinedSourceAddressArraySize(); ++k) {
                    // Encoded-Source Address
                    stream.writeByte(1);    // Address Family: IPv4
                    stream.writeByte(0);    // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
                    stream.writeNBitsOfUint64Be(0, 5);  // Reserved
                    stream.writeBit(pimJoinPrune->getJoinPruneGroups(i).getJoinedSourceAddress(k).S);
                    stream.writeBit(pimJoinPrune->getJoinPruneGroups(i).getJoinedSourceAddress(k).W);
                    stream.writeBit(pimJoinPrune->getJoinPruneGroups(i).getJoinedSourceAddress(k).R);
                    stream.writeByte(32);   // FIXME: Mask Length: 32 for IPv4 native encoding
                    stream.writeIpv4Address(pimJoinPrune->getJoinPruneGroups(i).getJoinedSourceAddress(k).IPaddress);
                }
                for (size_t k = 0; k < pimJoinPrune->getJoinPruneGroups(i).getPrunedSourceAddressArraySize(); ++k) {
                    // Encoded-Source Address
                    stream.writeByte(1);    // Address Family: IPv4
                    stream.writeByte(0);    // FIXME: Encoding Type: ?  The value '0' is reserved for this field and represents the native encoding of the Address Family.
                    stream.writeNBitsOfUint64Be(0, 5);  // Reserved
                    stream.writeBit(pimJoinPrune->getJoinPruneGroups(i).getPrunedSourceAddress(k).S);
                    stream.writeBit(pimJoinPrune->getJoinPruneGroups(i).getPrunedSourceAddress(k).W);
                    stream.writeBit(pimJoinPrune->getJoinPruneGroups(i).getPrunedSourceAddress(k).R);
                    stream.writeByte(32);   // FIXME: Mask Length: 32 for IPv4 native encoding
                    stream.writeIpv4Address(pimJoinPrune->getJoinPruneGroups(i).getPrunedSourceAddress(k).IPaddress);
                }
            }

            std::cout << "PimJoinPrune: length of the stream: " << stream.getLength() - start << endl;
            std::cout << "PimJoinPrune: chunkLength: " << pimJoinPrune->getChunkLength() << endl;
            break;
        }
        case Assert: {
            const auto& pimAssert = staticPtrCast<const PimAssert>(chunk);
            // Encoded-Group Address:
            stream.writeByte(1);    // Address Family: IPv4
            stream.writeByte(0);    // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.writeBit(0); // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
            stream.writeBitRepeatedly(0, 6);    // reserved
            stream.writeBit(0); // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
            stream.writeByte(32);   // FIXME: Mask Length: 32 for IPv4 native encoding
            stream.writeIpv4Address(pimAssert->getGroupAddress());
            // Encoded-Unicast Address
            stream.writeByte(1);    // Address Family: IPv4
            stream.writeByte(0);    // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.writeIpv4Address(pimAssert->getSourceAddress()); // address
            stream.writeBit(pimAssert->getR());
            stream.writeNBitsOfUint64Be(pimAssert->getMetricPreference(), 31);
            stream.writeUint32Be(pimAssert->getMetric());
            std::cout << "PimAssert: length of the stream: " << stream.getLength() - start << endl;
            std::cout << "PimAssert: chunkLength: " << pimAssert->getChunkLength() << endl;
            break;
        }
        case StateRefresh: {
            const auto& pimStateRefresh = staticPtrCast<const PimStateRefresh>(chunk);
            // Encoded-Group Address:
            stream.writeByte(1);    // Address Family: IPv4
            stream.writeByte(0);    // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.writeBit(0); // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
            stream.writeBitRepeatedly(0, 6);    // reserved
            stream.writeBit(0); // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
            stream.writeByte(32);   // FIXME: Mask Length: 32 for IPv4 native encoding
            stream.writeIpv4Address(pimStateRefresh->getGroupAddress());
            // Encoded-Unicast Address
            stream.writeByte(1);    // Address Family: IPv4
            stream.writeByte(0);    // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
            // address:
            stream.writeIpv4Address(pimStateRefresh->getSourceAddress());
            // Encoded-Unicast Address
            stream.writeByte(1);    // Address Family: IPv4
            stream.writeByte(0);    // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
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
            std::cout << "PimStateRefresh: length of the stream: " << stream.getLength() - start << endl;
            std::cout << "PimStateRefresh: chunkLength: " << pimStateRefresh->getChunkLength() << endl;
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
    stream.readUint16Be();  // FIXME: checksum??
    B length = B(4); // header length
    switch (pimPacket->getType()) {
        case Hello: {
            auto pimHello = makeShared<PimHello>();
            pimHello->setType(pimPacket->getType());
            pimHello->setVersion(pimPacket->getVersion());
            PimHelloOptionType type = static_cast<PimHelloOptionType>(stream.readUint16Be());
            size_t i = 0;
            while (type != static_cast<PimHelloOptionType>(10)) {
                switch (type) {
                    case Holdtime: {
                        uint16_t size = stream.readUint16Be();
                        ASSERT(size == 2);
                        pimHello->setOptionsArraySize(++i);
                        HoldtimeOption *holdtimeOption = new HoldtimeOption();
                        holdtimeOption->setHoldTime(stream.readUint16Be());
                        pimHello->setOptions(i - 1, holdtimeOption);
                        length += B(6);

                        type = static_cast<PimHelloOptionType>(stream.readUint16Be());
                        break;
                    }
                    case LANPruneDelay: {
                        uint16_t size = stream.readUint16Be();
                        ASSERT(size == 4);
                        stream.readBit();   // T bit
                        pimHello->setOptionsArraySize(++i);
                        LanPruneDelayOption *lanPruneDelayOption = new LanPruneDelayOption();
                        lanPruneDelayOption->setPropagationDelay(stream.readNBitsToUint64Be(15));
                        lanPruneDelayOption->setOverrideInterval(stream.readUint16Be());
                        pimHello->setOptions(i - 1, lanPruneDelayOption);
                        length += B(8);

                        type = static_cast<PimHelloOptionType>(stream.readUint16Be());
                        break;
                    }
                    case DRPriority: {
                        uint16_t size = stream.readUint16Be();
                        ASSERT(size == 4);
                        pimHello->setOptionsArraySize(++i);
                        DrPriorityOption *drPriorityOption = new DrPriorityOption();
                        drPriorityOption->setPriority(stream.readUint32Be());
                        pimHello->setOptions(i - 1, drPriorityOption);
                        length += B(8);

                        type = static_cast<PimHelloOptionType>(stream.readUint16Be());
                        break;
                    }
                    case GenerationID: {
                        uint16_t size = stream.readUint16Be();
                        ASSERT(size == 4);
                        pimHello->setOptionsArraySize(++i);
                        GenerationIdOption *generationIdOption = new GenerationIdOption();
                        generationIdOption->setGenerationID(stream.readUint32Be());
                        pimHello->setOptions(i - 1, generationIdOption);
                        length += B(8);

                        type = static_cast<PimHelloOptionType>(stream.readUint16Be());
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
            length += B(2);
            pimHello->setChunkLength(length);
            return pimHello;
        }
        case Register: {
            auto pimRegister = makeShared<PimRegister>();
            pimRegister->setType(pimPacket->getType());
            pimRegister->setVersion(pimPacket->getVersion());
            pimRegister->setB(stream.readBit());
            pimRegister->setN(stream.readBit());
            stream.readNBitsToUint64Be(30);
            //stream.readUint64Be();  // FIXME: Multicast data packet: ?
            length += B(4);
            pimRegister->setChunkLength(length);
            return pimRegister;
        }
        case RegisterStop: {
            auto pimRegisterStop = makeShared<PimRegisterStop>();
            pimRegisterStop->setType(pimPacket->getType());
            pimRegisterStop->setVersion(pimPacket->getVersion());
            // Encoded-Group Address:
            stream.readByte();  // Address Family: IPv4
            stream.readByte();  // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.readBit();   // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
            stream.readBitRepeatedly(0, 6); // reserved
            stream.readBit();   // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
            stream.readByte();  // FIXME: Mask Length: 32 for IPv4 native encoding
            pimRegisterStop->setGroupAddress(stream.readIpv4Address());
            // Encoded-Unicast Address
            stream.readByte();  // Address Family: IPv4
            stream.readByte();  // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
            pimRegisterStop->setSourceAddress(stream.readIpv4Address());
            length += (ENCODED_GROUP_ADDRESS_LENGTH + ENCODED_UNICODE_ADDRESS_LENGTH);
            pimRegisterStop->setChunkLength(length);
            return pimRegisterStop;
        }
        case Graft:
        case JoinPrune: {
            auto pimJoinPrune = makeShared<PimJoinPrune>();
            pimJoinPrune->setType(pimPacket->getType());
            pimJoinPrune->setVersion(pimPacket->getVersion());
            // Encoded-Unicast Address
            stream.readByte();  // Address Family: IPv4
            stream.readByte();  // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
            pimJoinPrune->setUpstreamNeighborAddress(stream.readIpv4Address());
            stream.readByte();
            pimJoinPrune->setJoinPruneGroupsArraySize(stream.readByte());
            pimJoinPrune->setHoldTime(stream.readUint16Be());
            length += (ENCODED_UNICODE_ADDRESS_LENGTH + B(4));
            if (pimPacket->getType() == Graft && pimJoinPrune->getHoldTime() != 0)
                pimJoinPrune->markIncorrect();
            for (size_t i = 0; i < pimJoinPrune->getJoinPruneGroupsArraySize(); ++i) {
                // Encoded-Group Address:
                stream.readByte();  // Address Family: IPv4
                stream.readByte();  // FIXME: Encoding Type: ? // The value '0' is reserved for this field and represents the native encoding of the Address Family.
                stream.readBit();   // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
                stream.readBitRepeatedly(0, 6); // reserved
                stream.readBit();   // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
                stream.readByte();  // FIXME: Mask Length: 32 for IPv4 native encoding
                pimJoinPrune->getJoinPruneGroupsForUpdate(i).setGroupAddress(stream.readIpv4Address());
                length += ENCODED_GROUP_ADDRESS_LENGTH;
                auto& joinPruneGroup = pimJoinPrune->getJoinPruneGroupsForUpdate(i);
                joinPruneGroup.setJoinedSourceAddressArraySize(stream.readUint16Be());
                joinPruneGroup.setPrunedSourceAddressArraySize(stream.readUint16Be());
                length += B(4);
                for (size_t k = 0; k < joinPruneGroup.getJoinedSourceAddressArraySize(); ++k) {
                    // Encoded-Source Address
                    stream.readByte();  // Address Family: IPv4
                    stream.readByte();  // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
                    stream.readNBitsToUint64Be(5);  // Reserved
                    auto& joinedSourceAddress = joinPruneGroup.getJoinedSourceAddressForUpdate(k);
                    joinedSourceAddress.S = stream.readBit();
                    joinedSourceAddress.W = stream.readBit();
                    joinedSourceAddress.R = stream.readBit();
                    stream.readByte();  // FIXME: Mask Length: 32 for IPv4 native encoding
                    joinedSourceAddress.IPaddress = stream.readIpv4Address();
                    length += ENCODED_SOURCE_ADDRESS_LENGTH;
                }
                for (size_t k = 0; k < pimJoinPrune->getJoinPruneGroups(i).getPrunedSourceAddressArraySize(); ++k) {
                    // Encoded-Source Address
                    stream.readByte();  // Address Family: IPv4
                    stream.readByte();  // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
                    stream.readNBitsToUint64Be(5);  // Reserved
                    auto& prunedSourceAddress = joinPruneGroup.getPrunedSourceAddressForUpdate(k);
                    prunedSourceAddress.S = stream.readBit();
                    prunedSourceAddress.W = stream.readBit();
                    prunedSourceAddress.R = stream.readBit();
                    stream.readByte();  // FIXME: Mask Length: 32 for IPv4 native encoding
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
            // Encoded-Group Address:
            stream.readByte();  // Address Family: IPv4
            stream.readByte();  // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.readBit();   // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
            stream.readBitRepeatedly(0, 6); // reserved
            stream.readBit();   // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
            stream.readByte();  // FIXME: Mask Length: 32 for IPv4 native encoding
            pimAssert->setGroupAddress(stream.readIpv4Address());
            // Encoded-Unicast Address
            stream.readByte();  // Address Family: IPv4
            stream.readByte();  // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
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
            // Encoded-Group Address:
            stream.readByte();  // Address Family: IPv4
            stream.readByte();  // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
            stream.readBit();   // FIXME: [B]idirectional PIM: Indicates the group range should use Bidirectional PIM.
            stream.readBitRepeatedly(0, 6); // reserved
            stream.readBit();   // FIXME: Admin Scope [Z]one: indicates the group range is an admin scope zone.
            stream.readByte();  // FIXME: Mask Length: 32 for IPv4 native encoding
            pimStateRefresh->setGroupAddress(stream.readIpv4Address());
            // Encoded-Unicast Address
            stream.readByte();  // Address Family: IPv4
            stream.readByte();  // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
            pimStateRefresh->setSourceAddress(stream.readIpv4Address());
            // Encoded-Unicast Address
            stream.readByte();  // Address Family: IPv4
            stream.readByte();  // FIXME: Encoding Type: ? The value '0' is reserved for this field and represents the native encoding of the Address Family.
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
            stream.readBit();
            // O:  Assert Override flag.  This SHOULD be set to 1 by upstream routers
            //     on a LAN if the Assert Timer (AT(S,G)) is not running and SHOULD be
            //     ignored upon receipt.
            stream.readBit();
            stream.readBitRepeatedly(0, 5);
            pimStateRefresh->setInterval(stream.readByte());
            length += (ENCODED_GROUP_ADDRESS_LENGTH + ENCODED_UNICODE_ADDRESS_LENGTH + ENCODED_UNICODE_ADDRESS_LENGTH + B(12));
            return pimStateRefresh;
        }
        default:
            throw cRuntimeError("Cannot serialize PIM packet: type %d not supported.", pimPacket->getType());
    }
}

} // namespace inet


















