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

#include "inet/common/packet/SerializerRegistry.h"
#include "inet/common/serializer/headerserializers/ethernet/EthernetHeaderSerializer.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/EtherPhyFrame.h"

namespace inet {

namespace serializer {

Register_Serializer(EtherFrame, EthernetMacHeaderSerializer);
Register_Serializer(EthernetFcs, EthernetFcsSerializer);
Register_Serializer(EtherPhyFrame, EthernetPhyHeaderSerializer);

void EthernetMacHeaderSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& ethernetHeader = std::static_pointer_cast<const EtherFrame>(chunk);
    stream.writeMACAddress(ethernetHeader->getDest());
    stream.writeMACAddress(ethernetHeader->getSrc());
    if (auto ethernetIIHeader = std::dynamic_pointer_cast<const EthernetIIFrame>(ethernetHeader)) {
        uint16_t ethType = ethernetIIHeader->getEtherType();
        stream.writeUint16(ethType);
    }
    else if (auto ethernetHeaderWithLLC = std::dynamic_pointer_cast<const EtherFrameWithLLC>(ethernetHeader)) {
        unsigned int payloadLengthPos = stream.getPosition();
        stream.writeUint16(0xFFFF);
        stream.writeByte(ethernetHeaderWithLLC->getSsap());
        stream.writeByte(ethernetHeaderWithLLC->getDsap());
        stream.writeByte(ethernetHeaderWithLLC->getControl());
        if (auto frame = std::dynamic_pointer_cast<const EtherFrameWithSNAP>(ethernetHeader)) {
            stream.writeByte(frame->getOrgCode() >> 16);
            stream.writeByte(frame->getOrgCode() >> 8);
            stream.writeByte(frame->getOrgCode());
            stream.writeUint16(frame->getLocalcode());
            // TODO:
//            unsigned int payloadLength = stream.getRemainingSize(4);
//            if (frame->getOrgCode() == 0) {
//                stream.writeUint16To(payloadLengthPos, payloadLength);
//            }
//            else {
//                //TODO
//                stream.writeUint16To(payloadLengthPos, payloadLength);
//            }
        }
// TODO:
//        else if (typeid(*frame) == typeid(EtherFrameWithLLC)) {
//            unsigned int payloadLength = stream.getRemainingSize(4);
//            stream.writeUint16To(payloadLengthPos, payloadLength);
//            SerializerBase::lookupAndSerialize(encapPkt, b, c, UNKNOWN, 0, payloadLength);
//        }
        else {
            throw cRuntimeError("Serializer not found for '%s'", ethernetHeader->getClassName());
        }
    }
    else if (auto pauseFrame = std::dynamic_pointer_cast<const EtherPauseFrame>(ethernetHeader)) {
        stream.writeUint16(0x8808);
        stream.writeUint16(0x0001);
        stream.writeUint16(pauseFrame->getPauseTime());
    }
    else {
        throw cRuntimeError("Cannot serialize '%s'", ethernetHeader->getClassName());
    }
    // TODO:
//    if (stream.getPosition() + 4 < ethernetHeader->getByteLength())
//        stream.fillNBytes((ethernetHeader->getByteLength() - 4) - stream.getPos(), 0);
//    uint32_t fcs = ethernetCRC(stream._getBuf(), stream.getPos());
//    stream.writeUint32(fcs);
}

std::shared_ptr<Chunk> EthernetMacHeaderSerializer::deserialize(ByteInputStream& stream) const
{
    int64_t position = stream.getPosition();
    std::shared_ptr<EtherFrame> ethernetMacHeader = nullptr;

    MACAddress destAddr = stream.readMACAddress();
    MACAddress srcAddr = stream.readMACAddress();
    uint16_t typeOrLength = stream.readUint16();

    // detect and create the real type
    if (typeOrLength >= 0x0600 || typeOrLength == 0) {
        auto ethernetIIHeader = std::make_shared<EthernetIIFrame>();
        ethernetIIHeader->setEtherType(typeOrLength);
        ethernetMacHeader = ethernetIIHeader;
    }
    else {
        std::shared_ptr<EtherFrameWithLLC> ethernetHeaderWithLLC = nullptr;
        uint8_t ssap = stream.readByte();
        uint8_t dsap = stream.readByte();
        uint8_t ctrl = stream.readByte();
        if (dsap == 0xAA && ssap == 0xAA) { // snap frame
            auto ethSnap = std::make_shared<EtherFrameWithSNAP>();
            ethSnap->setOrgCode(((uint32_t)stream.readByte() << 16) + stream.readUint16());
            ethSnap->setLocalcode(stream.readUint16());
            ethernetHeaderWithLLC = ethSnap;
        }
        else
            ethernetHeaderWithLLC = std::make_shared<EtherFrameWithLLC>();
        ethernetHeaderWithLLC->setDsap(dsap);
        ethernetHeaderWithLLC->setSsap(ssap);
        ethernetHeaderWithLLC->setControl(ctrl);
        ethernetMacHeader = ethernetHeaderWithLLC;
    }

    ethernetMacHeader->setDest(destAddr);
    ethernetMacHeader->setSrc(srcAddr);
    // TODO: ethernetMacHeader->setCrc(stream.readUint32());
    ethernetMacHeader->setChunkLength(byte(stream.getPosition() - position));
    return ethernetMacHeader;
}

void EthernetFcsSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    // TODO:
}

std::shared_ptr<Chunk> EthernetFcsSerializer::deserialize(ByteInputStream& stream) const
{
    // TODO:
    return nullptr;
}

void EthernetPhyHeaderSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    stream.writeByteRepeatedly(0x55, PREAMBLE_BYTES); // preamble
    stream.writeByte(0xD5); // SFD
}

std::shared_ptr<Chunk> EthernetPhyHeaderSerializer::deserialize(ByteInputStream& stream) const
{
    auto ethernetPhyHeader = std::make_shared<EtherPhyFrame>();
    bool preambleReadSuccessfully = stream.readByteRepeatedly(0x55, PREAMBLE_BYTES); // preamble
    uint8_t sfd = stream.readByte();
    if (!preambleReadSuccessfully || sfd != 0xD5) {
//        ethernetPhyHeader->markIncorrect();
//        ethernetPhyHeader->markImproperlyRepresented();
    }
//    return ethernetPhyHeader;
    return nullptr;
}

} // namespace serializer

} // namespace inet

