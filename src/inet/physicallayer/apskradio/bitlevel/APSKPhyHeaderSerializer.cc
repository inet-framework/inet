//
// Copyright (C) 2014 OpenSim Ltd.
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

#include "inet/common/serializer/EthernetCRC.h"
#include "inet/physicallayer/apskradio/bitlevel/APSKPhyHeaderSerializer.h"

namespace inet {

namespace physicallayer {

//Register_Serializer(APSKPhyHeader, APSKPhyHeaderSerializer);

void APSKPhyHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<Chunk>& chunk, int64_t offset, int64_t length) const
{

}

Ptr<Chunk> APSKPhyHeaderSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{

}

//BitVector *APSKPhyHeaderSerializer::serialize(const APSKPhyHeader *phyHeader) const
//{
//    const Ieee80211MacHeader *macFrame = check_and_cast<const Ieee80211MacHeader*>(phyHeader->getEncapsulatedPacket());
//    uint16_t macFrameLength = macFrame->getByteLength();
//    // KLUDGE: the serializer sometimes produces more or less bytes than the precomputed macFrameLength
//    unsigned char *buffer = new unsigned char[macFrameLength + 1000];
//    memset(buffer, 0, macFrameLength + 1000);
//    Ieee80211Serializer ieee80211Serializer;
//    Buffer b(buffer, macFrameLength);
//    Context c;
//    ieee80211Serializer.serializePacket(macFrame, b, c);
//    int serializedLength = b.getPos();
//    // TODO: ASSERT(serializedLength == macFrameLength);
//    uint32_t crc = ethernetCRC(buffer, serializedLength);
//    BitVector *bits = new BitVector();
//    bits->appendByte(serializedLength >> 8);
//    bits->appendByte(serializedLength >> 0);
//    bits->appendByte(crc >> 24);
//    bits->appendByte(crc >> 16);
//    bits->appendByte(crc >> 8);
//    bits->appendByte(crc >> 0);
//    for (int i = 0; i < serializedLength; i++)
//        bits->appendByte(buffer[i]);
//    delete[] buffer;
//    return bits;
//}
//
//APSKPhyHeader *APSKPhyHeaderSerializer::deserialize(const BitVector *bits) const
//{
//    const std::vector<uint8>& bytes = bits->getBytes();
//    APSKPhyHeader *phyHeader = new APSKPhyHeader();
//    cPacket *macFrame = nullptr;
//    if (bytes.size() < APSK_PHY_FRAME_HEADER_BYTE_LENGTH) {
//        macFrame = new cPacket();
//        phyHeader->setBitError(true);
//    }
//    else {
//        uint16_t macFrameLength = (bytes[0] << 8) + bytes[1];
//        uint32_t receivedCrc = (bytes[2] << 24) + (bytes[3] << 16) + (bytes[4] << 8) + bytes[5];
//        if (macFrameLength > bytes.size() - APSK_PHY_FRAME_HEADER_BYTE_LENGTH) {
//            macFrame = new cPacket();
//            phyHeader->setBitError(true);
//        }
//        else {
//            unsigned char *buffer = new unsigned char[macFrameLength];
//            for (unsigned int i = 0; i < macFrameLength; i++)
//                buffer[i] = bytes[i + APSK_PHY_FRAME_HEADER_BYTE_LENGTH];
//            uint32_t computedCrc = ethernetCRC(buffer, macFrameLength);
//            if (receivedCrc != computedCrc) {
//                EV_ERROR << "CRC check failed" << endl;
//                macFrame = new cPacket();
//                phyHeader->setBitError(true);
//            }
//            else {
//                Ieee80211Serializer deserializer;
//                Buffer b(buffer, macFrameLength);
//                Context c;
//                macFrame = deserializer.deserializePacket(b, c);
//            }
//            delete[] buffer;
//        }
//    }
//    phyHeader->setByteLength(APSK_PHY_FRAME_HEADER_BYTE_LENGTH);
//    phyHeader->encapsulate(macFrame);
//    return phyHeader;
//}

} // namespace physicallayer

} // namespace inet

