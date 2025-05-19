//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "QuicPacket.h"
#include "EncryptedQuicPacketChunk.h"
#include "EncryptionKeyTag_m.h"

namespace inet {
namespace quic {

QuicPacket::QuicPacket(std::string name) : name(name) { }

QuicPacket::~QuicPacket()
{
    for (QuicFrame *frame : frames) {
        delete frame;
    }
}

uint64_t QuicPacket::getPacketNumber()
{
    switch (header->getHeaderForm()) {
        case PACKET_HEADER_FORM_SHORT:
            return staticPtrCast<const ShortPacketHeader>(header)->getPacketNumber();
        case PACKET_HEADER_FORM_LONG:
            Ptr<const LongPacketHeader> longHeader = staticPtrCast<const LongPacketHeader>(header);
            switch (longHeader->getLongPacketType()) {
                case LONG_PACKET_HEADER_TYPE_INITIAL:
                    return staticPtrCast<const InitialPacketHeader>(header)->getPacketNumber();
                case LONG_PACKET_HEADER_TYPE_0RTT:
                    return staticPtrCast<const ZeroRttPacketHeader>(header)->getPacketNumber();
                case LONG_PACKET_HEADER_TYPE_HANDSHAKE:
                    return staticPtrCast<const HandshakePacketHeader>(header)->getPacketNumber();
                default:
                    cRuntimeError("getPacketNumber not implemented for this packet header type.");
            }
    }
    return 0;
}

bool QuicPacket::isCryptoPacket()
{
    return false;
}

void QuicPacket::setHeader(Ptr<PacketHeader> header)
{
    size += B(header->getChunkLength()).get();
    this->header = header;
}

void QuicPacket::addFrame(QuicFrame *frame)
{
    frames.push_back(frame);
    size += frame->getSize();
    dataSize += frame->getSize();
    if (frame->isAckEliciting()) {
        ackEliciting = true;
    }
    if (frame->countsAsInFlight()) {
        countsInFlight = true;
    }
}

Packet *QuicPacket::createOmnetPacket(const EncryptionKey& key)
{
    Ptr<SequenceChunk> encPayload = makeShared<SequenceChunk>();
    header->markImmutable();
    encPayload->insertAtBack(header);
    for (QuicFrame *frame : frames) {
        if (frame->getType() == FRAME_HEADER_TYPE_PADDING) {
            continue;
        }
        const_cast<FrameHeader *>(frame->getHeader().get())->markImmutable();
        encPayload->insertAtBack(frame->getHeader());
        if (frame->hasData()) {
            const_cast<Chunk *>(frame->getData().get())->markImmutable();
            encPayload->insertAtBack(frame->getData());
        }
    }

    encPayload->markImmutable();
    Ptr<EncryptedQuicPacketChunk> encPkt = makeShared<EncryptedQuicPacketChunk>(encPayload, encPayload->getChunkLength() + B(16));
    (*encPkt->addTag<EncryptionKeyTag>()) = *key.toTag();

    Packet *pkt = new Packet(name.c_str());
    pkt->insertAtBack(encPkt);

    for (QuicFrame *frame : frames) {
        if (frame->getType() != FRAME_HEADER_TYPE_PADDING) {
            continue;
        }
        const_cast<FrameHeader *>(frame->getHeader().get())->markImmutable();
        pkt->insertAtBack(frame->getHeader());
        ASSERT(!frame->hasData());
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
    staticPtrCast<OneRttPacketHeader>(header)->setIBit(iBit);
}

bool QuicPacket::isDplpmtudProbePacket()
{
    return false;
}

bool QuicPacket::containsFrame(QuicFrame *otherFrame)
{
    for (QuicFrame *frame : frames) {
        if (frame->equals(otherFrame)) {
            return true;
        }
    }
    return false;
}

int QuicPacket::getMemorySize()
{
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
