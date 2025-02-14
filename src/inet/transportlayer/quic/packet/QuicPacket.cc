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

#include "QuicPacket.h"

namespace inet {
namespace quic {

QuicPacket::QuicPacket(std::string name) {
    ackEliciting = false;
    countsInFlight = false;
    size = 0;
    dataSize = 0;
    this->name = name;
}

QuicPacket::~QuicPacket() {
    for (QuicFrame *frame : frames) {
        delete frame;
    }
}

uint64_t QuicPacket::getPacketNumber()
{
    switch (header->getPacketType()) {
        case PACKET_HEADER_TYPE_SHORT:
            return staticPtrCast<const ShortPacketHeader>(header)->getPacketNumber();
        default:
            cRuntimeError("getPacketNumber not implemented for packet header type other than short packet header.");
    }
    return 0;
}

bool QuicPacket::isCryptoPacket()
{
    return false;
}

void QuicPacket::setHeader(Ptr<PacketHeader> header) {
    size += B(header->getChunkLength()).get();
    this->header = header;
}

void QuicPacket::addFrame(QuicFrame *frame)
{
    frames.push_back(frame);
    size += frame->getSize();
    dataSize += frame->getDataSize();
    if (frame->isAckEliciting()) {
        ackEliciting = true;
    }
    if (frame->countsAsInFlight()) {
        countsInFlight = true;
    }
}

Packet *QuicPacket::createOmnetPacket()
{
    Packet *pkt = new Packet(name.c_str());
    pkt->insertAtBack(header);
    for (QuicFrame *frame : frames) {
        pkt->insertAtBack(frame->getHeader());
        if (frame->hasData()) {
            pkt->insertAtBack(frame->getData());
        }
    }
    return pkt;
}

void QuicPacket::onPacketLost()
{
    for (QuicFrame *frame : frames) {
        frame->onFrameLost();
    }
}

void QuicPacket::onPacketAcked()
{
    for (QuicFrame *frame : frames) {
        frame->onFrameAcked();
    }
}

void QuicPacket::setIBit(bool iBit)
{
    staticPtrCast<ShortPacketHeader>(header)->setIBit(iBit);
}

bool QuicPacket::isDplpmtudProbePacket()
{
    return false;
}

bool QuicPacket::containsFrame(QuicFrame *otherFrame) {
    for (QuicFrame *frame : frames) {
        if (frame->equals(otherFrame)) {
            return true;
        }
    }
    return false;
}

int QuicPacket::getMemorySize() {
    int size = sizeof(QuicPacket);
    if (header != nullptr) {
        size += sizeof(*header);
    }
    for (QuicFrame *frame : frames) {
        size += sizeof(frame);
        size += frame->getMemorySize();
    }
    return size;
}

} /* namespace quic */
} /* namespace inet */
