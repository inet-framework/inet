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

#include "platdep/sockets.h"  // htonl, ntohl, etc. on Windows
#include "inet/common/serializer/headerserializers/ethernet/EthernetSerializer.h"

#include "inet/common/serializer/headers/bsdint.h"
#include "inet/common/serializer/headers/ethernethdr.h"
#include "inet/common/serializer/headers/in.h"
#include "inet/common/serializer/headers/in_systm.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>  // htonl, ntohl, ...
#endif

#include "inet/common/serializer/headerserializers/EthernetCRC.h"

namespace inet {

namespace serializer {

Register_Serializer(EtherFrame, LINKTYPE, LINKTYPE_ETHERNET, EthernetSerializer);

void EthernetSerializer::serialize(const cPacket *pkt, Buffer &b, Context& c)
{
    ASSERT(b.getPos() == 0);
    const EtherFrame *ethPkt = check_and_cast<const EtherFrame *>(pkt);
    b.writeMACAddress(ethPkt->getDest());
    b.writeMACAddress(ethPkt->getSrc());
    if (dynamic_cast<const EthernetIIFrame *>(pkt)) {
        const EthernetIIFrame *frame = static_cast<const EthernetIIFrame *>(pkt);
        uint16_t ethType = frame->getEtherType();
        b.writeUint16(ethType);
        cPacket *encapPkt = frame->getEncapsulatedPacket();
        SerializerBase::serialize(encapPkt, b, c, ETHERTYPE, ethType, 4);
    }
    else if (dynamic_cast<const EtherFrameWithLLC *>(pkt)) {
        const EtherFrameWithLLC *frame = static_cast<const EtherFrameWithLLC *>(pkt);
        b.writeUint16(frame->getByteLength());
        b.writeByte(frame->getSsap());
        b.writeByte(frame->getDsap());
        b.writeByte(frame->getControl());
        if (dynamic_cast<const EtherFrameWithSNAP *>(pkt)) {
            const EtherFrameWithSNAP *frame = static_cast<const EtherFrameWithSNAP *>(pkt);
            b.writeByte(frame->getOrgCode() >> 16);
            b.writeByte(frame->getOrgCode() >> 8);
            b.writeByte(frame->getOrgCode());
            b.writeUint16(frame->getLocalcode());
            if (frame->getOrgCode() == 0) {
                cPacket *encapPkt = frame->getEncapsulatedPacket();
                SerializerBase::serialize(encapPkt, b, c, ETHERTYPE, frame->getLocalcode(), 4);
            }
            else {
                //TODO
                cPacket *encapPkt = frame->getEncapsulatedPacket();
                SerializerBase::serialize(encapPkt, b, c, UNKNOWN, frame->getLocalcode(), 4);
            }
        }
        else {
            throw cRuntimeError("Serializer not found for '%s'", pkt->getClassName());
        }
    }
    else if (dynamic_cast<const EtherPauseFrame *>(pkt)) {
        const EtherPauseFrame *frame = static_cast<const EtherPauseFrame *>(pkt);
        b.writeUint16(0x8808);
        b.writeUint16(0x0001);
        b.writeUint16(frame->getPauseTime());
    }
    else {
        throw cRuntimeError("Serializer not found for '%s'", pkt->getClassName());
    }
    if (b.getPos() + 4 < pkt->getByteLength())
        b.fillNBytes(pkt->getByteLength() - b.getPos() -4 , 0);
    uint32_t fcs = ethernetCRC(b._getBuf(), b.getPos());
    b.writeUint32(fcs);
}

cPacket* EthernetSerializer::parse(Buffer &b, Context& c)
{
    ASSERT(b.getPos() == 0);

    //FIXME should detect and create the real packet type.
    EthernetIIFrame *etherPacket = new EthernetIIFrame;

    etherPacket->setDest(b.readMACAddress());
    etherPacket->setSrc(b.readMACAddress());
    etherPacket->setEtherType(b.readUint16());

    cPacket *encapPacket = SerializerBase::parse(b, c, ETHERTYPE, etherPacket->getEtherType(), 4);
    ASSERT(encapPacket);
    etherPacket->encapsulate(encapPacket);
    etherPacket->setName(encapPacket->getName());
    if (b.getRemainder() > 4)
        b.accessNBytes(b.getRemainder() - 4);
    uint32_t calcfcs = ethernetCRC(b._getBuf(), b.getPos());
    uint32_t storedfcs = b.readUint32();
    if (storedfcs && calcfcs != 0xC704DD7B)
        etherPacket->setBitError(true);
    return etherPacket;
}

} // namespace serializer

} // namespace inet


