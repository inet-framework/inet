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

#include "inet/common/serializer/headerserializers/ieee80211/Ieee80211Serializer.h"
#include "inet/common/serializer/headerserializers/EthernetCRC.h"
#include "inet/physicallayer/apskradio/bitlevel/APSKPhyFrameSerializer.h"

namespace inet {

namespace physicallayer {

using namespace ieee80211;
using namespace inet::serializer;

APSKPhyFrameSerializer::APSKPhyFrameSerializer()
{
}

BitVector *APSKPhyFrameSerializer::serialize(const APSKPhyFrame *phyFrame) const
{
    const Ieee80211Frame *macFrame = check_and_cast<const Ieee80211Frame*>(phyFrame->getEncapsulatedPacket());
    uint16_t macFrameLength = macFrame->getByteLength();
    // KLUDGE: the serializer sometimes produces more or less bytes than the precomputed macFrameLength
    unsigned char *buffer = new unsigned char[macFrameLength + 1000];
    memset(buffer, 0, macFrameLength + 1000);
    Ieee80211Serializer ieee80211Serializer;
    Buffer b(buffer, macFrameLength);
    Context c;
    ieee80211Serializer.serializePacket(macFrame, b, c);
    int serializedLength = b.getPos();
    // TODO: ASSERT(serializedLength == macFrameLength);
    uint32_t crc = ethernetCRC(buffer, serializedLength);
    BitVector *bits = new BitVector();
    bits->appendByte(serializedLength >> 8);
    bits->appendByte(serializedLength >> 0);
    bits->appendByte(crc >> 24);
    bits->appendByte(crc >> 16);
    bits->appendByte(crc >> 8);
    bits->appendByte(crc >> 0);
    for (int i = 0; i < serializedLength; i++)
        bits->appendByte(buffer[i]);
    delete[] buffer;
    return bits;
}

APSKPhyFrame *APSKPhyFrameSerializer::deserialize(const BitVector *bits) const
{
    const std::vector<uint8>& bytes = bits->getBytes();
    APSKPhyFrame *phyFrame = new APSKPhyFrame();
    cPacket *macFrame = nullptr;
    if (bytes.size() < APSK_PHY_FRAME_HEADER_BYTE_LENGTH) {
        macFrame = new cPacket();
        phyFrame->setBitError(true);
    }
    else {
        uint16_t macFrameLength = (bytes[0] << 8) + bytes[1];
        uint32_t receivedCrc = (bytes[2] << 24) + (bytes[3] << 16) + (bytes[4] << 8) + bytes[5];
        if (macFrameLength > bytes.size() - APSK_PHY_FRAME_HEADER_BYTE_LENGTH) {
            macFrame = new cPacket();
            phyFrame->setBitError(true);
        }
        else {
            unsigned char *buffer = new unsigned char[macFrameLength];
            for (unsigned int i = 0; i < macFrameLength; i++)
                buffer[i] = bytes[i + APSK_PHY_FRAME_HEADER_BYTE_LENGTH];
            uint32_t computedCrc = ethernetCRC(buffer, macFrameLength);
            if (receivedCrc != computedCrc) {
                EV_ERROR << "CRC check failed" << endl;
                macFrame = new cPacket();
                phyFrame->setBitError(true);
            }
            else {
                Ieee80211Serializer deserializer;
                Buffer b(buffer, macFrameLength);
                Context c;
                macFrame = deserializer.deserializePacket(b, c);
            }
            delete[] buffer;
        }
    }
    phyFrame->setByteLength(APSK_PHY_FRAME_HEADER_BYTE_LENGTH);
    phyFrame->encapsulate(macFrame);
    return phyFrame;
}

} // namespace physicallayer

} // namespace inet

