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

#include "inet/common/BitVector.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/common/serializer/ieee80211/Ieee80211PhyHeaderSerializer.h"
#include "inet/common/serializer/ieee80211/Ieee80211PLCPHeaders.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMPLCPFrame_m.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211PLCPFrame_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"

namespace inet {

namespace serializer {

using namespace physicallayer;

Register_Serializer(Ieee80211PhyHeader, Ieee80211PhyHeaderSerializer);

void Ieee80211PhyHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<Chunk>& chunk) const
{
    const auto& phyHeader = std::static_pointer_cast<const Ieee80211PhyHeader>(chunk);
    // TODO:
    stream.writeByteRepeatedly('?', byte(phyHeader->getChunkLength()).get());

//    if (plcpHeader->getType() == OFDM)
//    {
//        const Ieee80211OFDMPLCPFrame *ofdmPhyFrame = check_and_cast<const Ieee80211OFDMPLCPFrame *>(plcpHeader);
//        unsigned int byteLength = ofdmPhyFrame->getByteLength(); // Byte length of the complete frame
//        unsigned char *buf = new unsigned char[byteLength];
//        for (unsigned int i = 0; i < byteLength; i++)
//            buf[i] = 0;
//        Buffer b(buf, byteLength);
//        Ieee80211OFDMPLCPHeader *hdr = (Ieee80211OFDMPLCPHeader *) b.accessNBytes(OFDM_PLCP_HEADER_LENGTH);
//        ASSERT(hdr);
//        hdr->length = ofdmPhyFrame->getLength(); // Byte length of the payload
//        hdr->rate = ofdmPhyFrame->getRate();
//        hdr->parity = 0; // TODO What is the correct value for this? Check the reference.
//        hdr->reserved = 0;
//        hdr->service = 0;
//        hdr->tail = 0;
//        Ieee80211MacHeader *encapsulatedPacket = check_and_cast<Ieee80211MacHeader*>(ofdmPhyFrame->getEncapsulatedPacket());
//        Ieee80211Serializer ieee80211Serializer;
//        // Here we just write the header which is exactly 5 bytes in length.
//        Buffer subBuffer(b, b.getRemainingSize());
//        Context c;
//        ieee80211Serializer.serializePacket(encapsulatedPacket, subBuffer, c);
//        b.accessNBytes(subBuffer.getPos());
//        unsigned int numOfWrittenBytes = b.getPos();
//        // TODO: This assertion must hold!
////        ASSERT(numOfWrittenBytes == byteLength);
//        writeToBitVector(buf, numOfWrittenBytes, serializedPacket);
//        // KLUDGE: if numOfWrittenBytes != byteLength it causes runtime error at the physical layer
//        int pad = byteLength - numOfWrittenBytes;
//        if (pad > 0)
//            serializedPacket->appendBit(false, pad * 8);
//        serializedPacket->appendBit(0, 6); // tail bits
//        delete[] buf;
//        return true;
//    }
//    return false;
}

Ptr<Chunk> Ieee80211PhyHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto phyHeader = std::make_shared<Ieee80211PhyHeader>();
    // TODO:
    phyHeader->setChunkLength(bit(192));
    stream.readByteRepeatedly('?', byte(phyHeader->getChunkLength()).get());

//    // TODO: Revise this code snippet and optimize
//    // FIXME: We only have OFDM deserializer
//    unsigned char *hdrBuf = new unsigned char[OFDM_PLCP_HEADER_LENGTH];
//    const std::vector<uint8>& bitFields = serializedPacket->getBytes();
//    for (int i = 0; i < OFDM_PLCP_HEADER_LENGTH; i++)
//        hdrBuf[i] = bitFields[i];
//    Ieee80211OFDMPLCPHeader *hdr = (Ieee80211OFDMPLCPHeader *) hdrBuf;
//    Ieee80211OFDMPLCPFrame *plcpFrame = new Ieee80211OFDMPLCPFrame();
//    plcpFrame->setRate(hdr->rate);
//    plcpFrame->setLength(hdr->length);
//    plcpFrame->setType(OFDM);
//    unsigned char *buf = new unsigned char[hdr->length + OFDM_PLCP_HEADER_LENGTH];
//    for (int i = 0; i < hdr->length + OFDM_PLCP_HEADER_LENGTH; i++)
//        buf[i] = bitFields[i];
//    Ieee80211Serializer serializer;
//    Buffer subBuffer(buf + OFDM_PLCP_HEADER_LENGTH, hdr->length);
//    Context c;
//    cPacket *payload = serializer.deserializePacket(subBuffer, c);
//    plcpFrame->setBitLength(OFDM_PLCP_HEADER_LENGTH);
//    plcpFrame->encapsulate(payload);
////    ASSERT(plcpFrame->getBitLength() == OFDM_PLCP_HEADER_LENGTH + 8 * hdr->length);
//    delete[] buf;
//    delete[] hdrBuf;
//    return plcpFrame;
    return phyHeader;
}

//void Ieee80211PhySerializer::writeToBitVector(unsigned char* buf, unsigned int bufSize, BitVector* bitVector) const
//{
//    for (unsigned int i = 0; i < bufSize; i++)
//    {
//        unsigned int currentByte = buf[i];
//        for (unsigned int j = 0; j < 8; j++)
//        {
//            bool currentBit = currentByte & (1 << j);
//            bitVector->appendBit(currentBit);
//        }
//    }
//}

} // namespace serializer

} // namespace inet

