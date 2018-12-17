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
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/linklayer/ethernet/EthernetHeaderSerializer.h"

namespace inet {

Register_Serializer(EthernetMacHeader, EthernetMacHeaderSerializer);
Register_Serializer(EthernetControlFrame, EthernetControlFrameSerializer);
Register_Serializer(EthernetPauseFrame, EthernetControlFrameSerializer);
Register_Serializer(EthernetPadding, EthernetPaddingSerializer);
Register_Serializer(EthernetFcs, EthernetFcsSerializer);
Register_Serializer(EthernetPhyHeader, EthernetPhyHeaderSerializer);

static void serializeQtag(MemoryOutputStream& stream, uint16_t ethType, const Ieee8021qHeader *qtag)
{
    stream.writeUint16Be(ethType);
    stream.writeUint16Be((qtag->getVid() & 0xFFF) |
                         ((qtag->getPcp() & 7) << 13) |
                         (qtag->getDe() ? 0x1000 : 0));
}

static Ieee8021qHeader *deserializeQtag(MemoryInputStream& stream)
{
    auto qtag = new Ieee8021qHeader();
    uint16_t qtagValue = stream.readUint16Be();
    qtag->setVid(qtagValue & 0xFFF);
    qtag->setPcp((qtagValue >> 13) & 7);
    qtag->setDe((qtagValue & 0x1000) != 0);
    return qtag;
}

void EthernetMacHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& ethernetMacHeader = staticPtrCast<const EthernetMacHeader>(chunk);
    stream.writeMacAddress(ethernetMacHeader->getDest());
    stream.writeMacAddress(ethernetMacHeader->getSrc());
    if (ethernetMacHeader->getSTag() != nullptr)
        serializeQtag(stream, 0x88A8, ethernetMacHeader->getSTag());
    if (ethernetMacHeader->getCTag() != nullptr)
        serializeQtag(stream, 0x8100, ethernetMacHeader->getCTag());
    stream.writeUint16Be(ethernetMacHeader->getTypeOrLength());
}

const Ptr<Chunk> EthernetMacHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    Ptr<EthernetMacHeader> ethernetMacHeader = makeShared<EthernetMacHeader>();
    MacAddress destAddr = stream.readMacAddress();
    MacAddress srcAddr = stream.readMacAddress();
    uint16_t value = stream.readUint16Be();
    ethernetMacHeader->setDest(destAddr);
    ethernetMacHeader->setSrc(srcAddr);
    if (value == 0x88A8) {
        ethernetMacHeader->setSTag(deserializeQtag(stream));
        value = stream.readUint16Be();
    }
    if (value == 0x8100) {
        ethernetMacHeader->setCTag(deserializeQtag(stream));
        value = stream.readUint16Be();
    }
    ethernetMacHeader->setTypeOrLength(value);
    return ethernetMacHeader;
}

void EthernetControlFrameSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& frame = staticPtrCast<const EthernetControlFrame>(chunk);
    stream.writeUint16Be(frame->getOpCode());
    if (frame->getOpCode() == ETHERNET_CONTROL_PAUSE) {
        auto pauseFrame = dynamicPtrCast<const EthernetPauseFrame>(frame);
        ASSERT(pauseFrame != nullptr);
        stream.writeUint16Be(pauseFrame->getPauseTime());
    }
    else
        throw cRuntimeError("Cannot serialize '%s' (EthernetControlFrame with opCode = %d)", frame->getClassName(), frame->getOpCode());
}

const Ptr<Chunk> EthernetControlFrameSerializer::deserialize(MemoryInputStream& stream) const
{
    Ptr<EthernetControlFrame> controlFrame = nullptr;
    uint16_t opCode = stream.readUint16Be();
    if (opCode == ETHERNET_CONTROL_PAUSE) {
        auto pauseFrame = makeShared<EthernetPauseFrame>();
        pauseFrame->setOpCode(opCode);
        pauseFrame->setPauseTime(stream.readUint16Be());
        controlFrame = pauseFrame;
    }
    else {
        controlFrame = makeShared<EthernetControlFrame>();
        controlFrame->setOpCode(opCode);
        controlFrame->markImproperlyRepresented();
    }
    return controlFrame;
}

void EthernetPaddingSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    stream.writeByteRepeatedly(0, B(chunk->getChunkLength()).get());
}

const Ptr<Chunk> EthernetPaddingSerializer::deserialize(MemoryInputStream& stream) const
{
    throw cRuntimeError("Invalid operation");
}

void EthernetFcsSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& ethernetFcs = staticPtrCast<const EthernetFcs>(chunk);
    if (ethernetFcs->getFcsMode() != FCS_COMPUTED)
        throw cRuntimeError("Cannot serialize Ethernet FCS without a properly computed FCS");
    stream.writeUint32Be(ethernetFcs->getFcs());
}

const Ptr<Chunk> EthernetFcsSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ethernetFcs = makeShared<EthernetFcs>();
    ethernetFcs->setFcs(stream.readUint32Be());
    ethernetFcs->setFcsMode(FCS_COMPUTED);
    return ethernetFcs;
}

void EthernetPhyHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    stream.writeByteRepeatedly(0x55, B(PREAMBLE_BYTES).get()); // preamble
    stream.writeByte(0xD5); // SFD
}

const Ptr<Chunk> EthernetPhyHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ethernetPhyHeader = makeShared<EthernetPhyHeader>();
    bool preambleReadSuccessfully = stream.readByteRepeatedly(0x55, B(PREAMBLE_BYTES).get()); // preamble
    uint8_t sfd = stream.readByte();
    if (!preambleReadSuccessfully || sfd != 0xD5)
        ethernetPhyHeader->markIncorrect();
    return ethernetPhyHeader;
}

} // namespace inet

