//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/acking/AckingMacHeader_m.h"
#include "inet/linklayer/acking/AckingMacHeaderSerializer.h"

#ifdef WITH_ETHERNET
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#endif

namespace inet {

Register_Serializer(AckingMacHeader, AckingMacHeaderSerializer);

void AckingMacHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    auto startPosition = stream.getLength();
    auto macHeader = staticPtrCast<const AckingMacHeader>(chunk);
    stream.writeUint8(B(macHeader->getChunkLength()).get());
    stream.writeMacAddress(macHeader->getSrc());
    stream.writeMacAddress(macHeader->getDest());
    stream.writeUint16Be(macHeader->getNetworkProtocol());
    stream.writeUint64Be(macHeader->getSrcModuleId());
    int64_t remainders = B(macHeader->getChunkLength() - (stream.getLength() - startPosition)).get();
    if (remainders < 0)
        throw cRuntimeError("AckingMacHeader length = %d smaller than required %d bytes", (int)B(macHeader->getChunkLength()).get(), (int)B(stream.getLength() - startPosition).get());
    stream.writeByteRepeatedly('?', remainders);
}

const Ptr<Chunk> AckingMacHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto startPosition = stream.getPosition();
    auto macHeader = makeShared<AckingMacHeader>();
    uint8_t length = stream.readUint8();
    macHeader->setChunkLength(B(length));
    macHeader->setSrc(stream.readMacAddress());
    macHeader->setDest(stream.readMacAddress());
    macHeader->setNetworkProtocol(stream.readUint16Be());
    macHeader->setSrcModuleId(stream.readUint64Be());
    B remainders = B(length) - (stream.getPosition() - startPosition);
    ASSERT(remainders >= B(0));
    stream.readByteRepeatedly('?', B(remainders).get());
    return macHeader;
}

Register_Class(AckingMacToEthernetPcapRecorderHelper);

bool AckingMacToEthernetPcapRecorderHelper::matchesLinkType(PcapLinkType pcapLinkType, const Protocol *protocol) const
{
    return false;
}

PcapLinkType AckingMacToEthernetPcapRecorderHelper::protocolToLinkType(const Protocol *protocol) const
{
#if defined(WITH_ETHERNET)
    if (*protocol == Protocol::ackingMac)
        return LINKTYPE_ETHERNET;
#endif // defined(WITH_ETHERNET)
    return LINKTYPE_INVALID;
}

Packet *AckingMacToEthernetPcapRecorderHelper::tryConvertToLinkType(const Packet* packet, PcapLinkType pcapLinkType, const Protocol *protocol) const
{
#if defined(WITH_ETHERNET)
    if (*protocol == Protocol::ackingMac && pcapLinkType == LINKTYPE_ETHERNET) {
        auto newPacket = packet->dup();
        auto ackingHdr = newPacket->popAtFront<AckingMacHeader>();
        newPacket->trimFront();
        auto ethHeader = makeShared<EthernetMacHeader>();
        ethHeader->setDest(ackingHdr->getDest());
        ethHeader->setSrc(ackingHdr->getSrc());
        ethHeader->setTypeOrLength(ackingHdr->getNetworkProtocol());
        newPacket->insertAtFront(ethHeader);
        newPacket->getTagForUpdate<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
        return newPacket;
    }
#endif // defined(WITH_ETHERNET)

    return nullptr;
}

} // namespace inet

