//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "PacketBuilder.h"
#include "packet/PacketHeader_m.h"
#include "inet/common/Units.h"
#include "packet/QuicStreamFrame.h"

extern "C" {
#include "picotls.h"
#include "picotls/openssl.h"
}
namespace inet {
namespace quic {

PacketBuilder::PacketBuilder(std::vector<QuicFrame*> *controlQueue, IScheduler *scheduler, ReceivedPacketsAccountant *receivedPacketsAccountant[]) {
    this->controlQueue = controlQueue;
    this->scheduler = scheduler;
    this->receivedPacketsAccountant = receivedPacketsAccountant;
    bundleAckForNonAckElicitingPackets = false;
    packetNumber[PacketNumberSpace::ApplicationData] = 0;
    packetNumber[PacketNumberSpace::Handshake] = 0;
    packetNumber[PacketNumberSpace::Initial] = 0;
}

PacketBuilder::~PacketBuilder() { }

void PacketBuilder::readParameters(cModule *module)
{
    // the parameter bundleAckForNonAckElicitingPackets tells us whether to generate an ack upon
    // a non-ack-eliciting packet, if we are able to bundle it into an ack-eliciting outgoing packet.
    this->bundleAckForNonAckElicitingPackets = module->par("bundleAckForNonAckElicitingPackets");

    this->skipPacketNumberForDplpmtudProbePackets = module->par("skipPacketNumberForDplpmtudProbePackets");
}

void PacketBuilder::fillLongHeader(Ptr<LongPacketHeader> packetHeader)
{
    ASSERT(srcConnectionId != nullptr);
    packetHeader->setSrcConnectionIdLength(srcConnectionId->getLength());
    packetHeader->setSrcConnectionId(srcConnectionId->getId());
    ASSERT(dstConnectionId != nullptr);
    /*
    if (dstConnectionId == nullptr) {
        packetHeader->setDstConnectionIdLength(8);
        packetHeader->setDstConnectionId(0x1122334455667788); // TODO: should be random
    } else {
    */
        packetHeader->setDstConnectionIdLength(dstConnectionId->getLength());
        packetHeader->setDstConnectionId(dstConnectionId->getId());
    //}
}

Ptr<InitialPacketHeader> PacketBuilder::createInitialHeader()
{
    Ptr<InitialPacketHeader> packetHeader = makeShared<InitialPacketHeader>();
    fillLongHeader(packetHeader);
    packetHeader->setPacketNumberLength(1);
    packetHeader->setPacketNumber(packetNumber[PacketNumberSpace::Initial]++);
    packetHeader->setTokenLength(0);
    packetHeader->setToken(0);
    packetHeader->setLength(1);
    packetHeader->calcChunkLength();

    return packetHeader;
}

Ptr<HandshakePacketHeader> PacketBuilder::createHandshakeHeader()
{
    Ptr<HandshakePacketHeader> packetHeader = makeShared<HandshakePacketHeader>();
    fillLongHeader(packetHeader);
    packetHeader->setPacketNumberLength(1);
    packetHeader->setPacketNumber(packetNumber[PacketNumberSpace::Handshake]++);
    packetHeader->setLength(1);
    packetHeader->calcChunkLength();

    return packetHeader;
}

Ptr<OneRttPacketHeader> PacketBuilder::createOneRttHeader()
{
    // create short header
    Ptr<OneRttPacketHeader> packetHeader = makeShared<OneRttPacketHeader>();
    packetHeader->setPacketNumber(packetNumber[PacketNumberSpace::ApplicationData]++);
    ASSERT(dstConnectionId != nullptr);
    packetHeader->setDstConnectionId(dstConnectionId->getId());
    // TODO: set appropriate chunk length --> actual packet header size according to the spec
    packetHeader->setChunkLength(B(OneRttPacketHeader::SIZE));

    return packetHeader;
}

Ptr<ZeroRttPacketHeader> PacketBuilder::createZeroRttHeader()
{
    Ptr<ZeroRttPacketHeader> packetHeader = makeShared<ZeroRttPacketHeader>();
    fillLongHeader(packetHeader);
    packetHeader->setPacketNumberLength(1);
    packetHeader->setPacketNumber(packetNumber[PacketNumberSpace::ApplicationData]++);
    packetHeader->setLength(1);
    packetHeader->calcChunkLength();

    return packetHeader;
}

QuicPacket *PacketBuilder::createPacket(PacketNumberSpace pnSpace, bool skipPacketNumber, bool zeroRtt)
{
    if (skipPacketNumber) {
        packetNumber[pnSpace]++;
    }
    Ptr<PacketHeader> header;
    std::stringstream packetName;
    switch (pnSpace) {
        case PacketNumberSpace::Initial:
            packetName << "Initial[" << packetNumber[pnSpace] << "]";
            header = createInitialHeader();
            break;
        case PacketNumberSpace::Handshake:
            packetName << "Handshake[" << packetNumber[pnSpace] << "]";
            header = createHandshakeHeader();
            break;
        case PacketNumberSpace::ApplicationData:
            if (zeroRtt) {
                packetName << "0-RTT[" << packetNumber[pnSpace] << "]";
                header = createZeroRttHeader();
            } else {
                packetName << "1-RTT[" << packetNumber[pnSpace] << "]";
                header = createOneRttHeader();
            }
            break;
    }
    QuicPacket *packet = new QuicPacket(packetName.str());
    packet->setHeader(header);

    return packet;
}

QuicPacket *PacketBuilder::createOneRttPacket(bool skipPacketNumber)
{
    return createPacket(PacketNumberSpace::ApplicationData, skipPacketNumber);
}

QuicPacket *PacketBuilder::createZeroRttPacket()
{
    return createPacket(PacketNumberSpace::ApplicationData, false, true);
}

QuicFrame *PacketBuilder::createPingFrame()
{
    Ptr<PingFrameHeader> frameHeader = makeShared<PingFrameHeader>();
    return new QuicFrame(frameHeader);
}

QuicFrame *PacketBuilder::createPaddingFrame(int length)
{
    Ptr<PaddingFrameHeader> frameHeader = makeShared<PaddingFrameHeader>();
    // By specification, a PADDING frame is one byte in size.
    // However, instead of adding multiple PADDING frames, for simulation,
    // QUIC adds one and adjusts its length
    frameHeader->setLength(length);
    return new QuicFrame(frameHeader);
}

QuicFrame *PacketBuilder::createCryptoFrame(TransportParameters *tp)
{
    Ptr<CryptoFrameHeader> frameHeader = makeShared<CryptoFrameHeader>();

    QuicFrame *frame = new QuicFrame(frameHeader);

    if (tp) {


        ptls_context_t ctx;

        memset(&ctx, 0, sizeof(ctx));
        ctx.random_bytes = ptls_openssl_random_bytes;
        ctx.key_exchanges = ptls_openssl_key_exchanges;
        ctx.cipher_suites = ptls_openssl_cipher_suites;
        ctx.get_time = &ptls_get_time;


        const int EXTENSION_TYPE_TRANSPORT_PARAMETERS_FINAL = 0x39;
        const int TRANSPORT_PARAMETER_ID_INITIAL_MAX_DATA = 4;
        const int TRANSPORT_PARAMETER_ID_INITIAL_MAX_STREAM_DATA_BIDI_LOCAL = 5;
        const int TRANSPORT_PARAMETER_ID_INITIAL_MAX_STREAM_DATA_BIDI_REMOTE = 6;
        const int TRANSPORT_PARAMETER_ID_INITIAL_MAX_STREAM_DATA_UNI = 7;
        int ret;

#define PUSH_TP(buf, id, block)                                                                                                    \
    do {                                                                                                                           \
        ptls_buffer_push_quicint((buf), (id));                                                                                     \
        ptls_buffer_push_block((buf), -1, block);                                                                                  \
    } while (0)

        ptls_buffer_t buf;

        ptls_buffer_init(&buf, (void*)"", 0);

        PUSH_TP(&buf, TRANSPORT_PARAMETER_ID_INITIAL_MAX_DATA, { ptls_buffer_push_quicint(&buf, tp->initialMaxData); });
        PUSH_TP(&buf, TRANSPORT_PARAMETER_ID_INITIAL_MAX_STREAM_DATA_BIDI_LOCAL,
                { ptls_buffer_push_quicint(&buf, tp->initialMaxStreamData); });
        PUSH_TP(&buf, TRANSPORT_PARAMETER_ID_INITIAL_MAX_STREAM_DATA_BIDI_REMOTE,
                { ptls_buffer_push_quicint(&buf, tp->initialMaxStreamData); });
        PUSH_TP(&buf, TRANSPORT_PARAMETER_ID_INITIAL_MAX_STREAM_DATA_UNI,
                { ptls_buffer_push_quicint(&buf, tp->initialMaxStreamData); });

        struct {
            ptls_raw_extension_t ext[2];
            ptls_buffer_t buf;
        } transport_params;

        transport_params.ext[0] =
            (ptls_raw_extension_t){EXTENSION_TYPE_TRANSPORT_PARAMETERS_FINAL,
                                   {buf.base, buf.off}};
        transport_params.ext[1] = (ptls_raw_extension_t){UINT16_MAX};

        ptls_handshake_properties_t handshake_properties;

        memset(&handshake_properties, 0, sizeof(handshake_properties));
        handshake_properties.additional_extensions = transport_params.ext;

        size_t epoch_offsets[5] = {0};

        ptls_buffer_t buf2;
        ptls_buffer_init(&buf2, (void*)"", 0);

        ptls_t *tls;
        tls = ptls_new(&ctx, 0);

        ptls_handle_message(tls, &buf2, epoch_offsets, 0, NULL, 0, &handshake_properties);

        if (buf2.off == 0)
            return 0;

        for (size_t epoch = 0; epoch < 4; ++epoch) {
            size_t len = epoch_offsets[epoch + 1] - epoch_offsets[epoch];
            if (len == 0)
                continue;

            Ptr<BytesChunk> transportParametersExt = makeShared<BytesChunk>();
            std::vector<uint8_t> bytes;
            bytes.resize(len);
            for (size_t i = 0; i < len; i++) {
                bytes[i] = *(uint8_t *)(buf2.base + epoch_offsets[epoch] + i);
            }
            transportParametersExt->setBytes(bytes);
            frame->setData(transportParametersExt);
        }

        frameHeader->setContainsTransportParameters(true);

    }

    frameHeader->setOffset(0);
    frameHeader->setLength(frame->getDataSize());
    frameHeader->calcChunkLength();

    Exit:
    return frame;
}

QuicFrame *PacketBuilder::createConnectionCloseFrame(bool appInitiated, int errorCode)
{
    Ptr<ConnectionCloseFrameHeader> frameHeader = makeShared<ConnectionCloseFrameHeader>();
    if (appInitiated) {
        frameHeader->setFrameType(FRAME_HEADER_TYPE_CONNECTION_CLOSE_APP);
    } else {
        frameHeader->setFrameType(FRAME_HEADER_TYPE_CONNECTION_CLOSE_QUIC);
    }
    frameHeader->setErrorCode(errorCode);
    frameHeader->calcChunkLength();
    return new QuicFrame(frameHeader);
}

QuicFrame *PacketBuilder::createHandshakeDoneFrame()
{
    Ptr<HandshakeDoneFrameHeader> frameHeader = makeShared<HandshakeDoneFrameHeader>();
    return new QuicFrame(frameHeader);
}

QuicFrame *PacketBuilder::createNewTokenFrame(uint32_t token)
{
    Ptr<NewTokenFrameHeader> frameHeader = makeShared<NewTokenFrameHeader>();
    frameHeader->setToken(token);
    frameHeader->calcChunkLength();
    return new QuicFrame(frameHeader);
}

QuicPacket *PacketBuilder::addFramesFromControlQueue(QuicPacket *packet, int maxPacketSize)
{
    while (!controlQueue->empty()) {

        auto controlFrame = controlQueue->front();
        auto controlFrameHeader = controlFrame->getHeader();
        auto controlFrameSize = B(controlFrameHeader->getChunkLength()).get();

        if ((maxPacketSize - getPacketSize(packet)) < controlFrameSize) {
            return packet;
        }

        EV_DEBUG << "insert control frame" << controlFrame->getType()  << endl;

        packet = addFrameToPacket(packet, controlFrame);
        controlQueue->erase(controlQueue->begin());
    }
    return packet;
}

QuicPacket *PacketBuilder::addFrameToPacket(QuicPacket *packet, QuicFrame *frame, bool skipPacketNumber)
{
    if (packet == nullptr) {
        packet = createOneRttPacket(skipPacketNumber);
    }
    packet->addFrame(frame);
    return packet;
}

size_t PacketBuilder::getPacketSize(QuicPacket *packet)
{
    if (packet == nullptr) {
        return OneRttPacketHeader::SIZE;
    }
    return packet->getSize();
}

QuicPacket *PacketBuilder::buildPacket(int maxPacketSize, int safePacketSize)
{
    QuicPacket *packet = nullptr;

    // add ack frame first, if need to send one
    if (receivedPacketsAccountant[PacketNumberSpace::ApplicationData]->wantsToSendAckImmediately()) {
        // restrict packet to a safe size, because this might get an non-ack-eliciting packet; PMTU validation works only with ack-eliciting packets
        QuicFrame *ackFrame = receivedPacketsAccountant[PacketNumberSpace::ApplicationData]->generateAckFrame(safePacketSize - getPacketSize(packet));
        if (ackFrame == nullptr) {
            throw cRuntimeError("buildPacket: max packet size too small, not even an Ack frame fits into it.");
        }
        packet = addFrameToPacket(packet, ackFrame);
    }

    //First add frames from controlQueue in Packet
    packet = addFramesFromControlQueue(packet, maxPacketSize);

    // check if we would like to bundle an ack frame
    if (receivedPacketsAccountant[PacketNumberSpace::ApplicationData]->hasNewAckInfoAboutAckElicitings() || (receivedPacketsAccountant[PacketNumberSpace::ApplicationData]->hasNewAckInfo() && bundleAckForNonAckElicitingPackets)) {
        // if the packet is already ack-eliciting (due to a control frame), add an ack frame
        if (packet != nullptr && packet->isAckEliciting()) {
            QuicFrame *ackFrame = receivedPacketsAccountant[PacketNumberSpace::ApplicationData]->generateAckFrame(maxPacketSize - getPacketSize(packet));
            if (ackFrame != nullptr) {
                packet = addFrameToPacket(packet, ackFrame);
            }
        }
    }

    // select stream for STREAM frame
    Stream *selectedStream = nullptr;
    //int streamDataSize = scheduler->selectStream(&selectedStream, maxPacketSize - getPacketSize(packet));
    selectedStream = scheduler->selectStream(maxPacketSize - getPacketSize(packet));
    // if remainingPacketSize is too small or there is no stream queue with data, selectStream should return 0.

    //check if controlQueue has got ControlFrames after selectStream due to FlowControl
    packet = addFramesFromControlQueue(packet, maxPacketSize);

    // check if we would like to bundle an ack frame
    if (receivedPacketsAccountant[PacketNumberSpace::ApplicationData]->hasNewAckInfoAboutAckElicitings() || (receivedPacketsAccountant[PacketNumberSpace::ApplicationData]->hasNewAckInfo() && bundleAckForNonAckElicitingPackets)) {
        QuicFrame *ackFrame = nullptr;
        if (packet != nullptr && packet->isAckEliciting()) {
            // packet is already ack eliciting, we can use the full remaining space for the ack.
            ackFrame = receivedPacketsAccountant[PacketNumberSpace::ApplicationData]->generateAckFrame(maxPacketSize - getPacketSize(packet));
        } else if (selectedStream != nullptr) {
            // packet is not ack-eliciting now, but becomes ack-eliciting when we add the STREAM frame.
            // we have keep the ack frame small enough, such that at least one STREAM frame (with at least one byte of data) fits into the packet.
            ackFrame = receivedPacketsAccountant[PacketNumberSpace::ApplicationData]->generateAckFrame(maxPacketSize - getPacketSize(packet) - selectedStream->getNextStreamFrameSize(1));
        }

        if (ackFrame != nullptr) {
            packet = addFrameToPacket(packet, ackFrame);

            // if we are going to add a stream frame, we might have to reduce the streamDataSize, now (after we added the ACK frame).
            if (selectedStream != nullptr) {
                if (selectedStream->getNextStreamFrameSize(maxPacketSize - getPacketSize(packet)) == 0) {
                    selectedStream = nullptr;
                }
            }
        }
    }

    while (selectedStream != nullptr) {
        // is enough space remaining for a stream frame header with at least one byte of data?
        //if (remainingPacketSize < selectedStream->getHeaderSize() + streamDataSize) {
        //    return pkt;
        //}

        packet = addFrameToPacket(packet, selectedStream->generateNextStreamFrame(maxPacketSize - getPacketSize(packet)));

        selectedStream = scheduler->selectStream(maxPacketSize - getPacketSize(packet));

        //check if controlQueue has got ControlFrames after selectStream due to FlowControl
        packet = addFramesFromControlQueue(packet, maxPacketSize);
    }

    ASSERT(packet == nullptr || packet->getSize() <= maxPacketSize);
    return packet;

}

QuicPacket *PacketBuilder::buildAckOnlyPacket(int maxPacketSize, PacketNumberSpace pnSpace)
{
    QuicPacket *packet = createPacket(pnSpace, false);

    QuicFrame *ackFrame = receivedPacketsAccountant[pnSpace]->generateAckFrame(maxPacketSize - getPacketSize(packet));
    if (ackFrame == nullptr) {
        throw cRuntimeError("buildAckOnlyPacket: max packet size too small, not even an Ack frame fits into it.");
    }
    packet->addFrame(ackFrame);

    ASSERT(packet != nullptr && packet->getSize() <= maxPacketSize);
    return packet;
}

QuicPacket *PacketBuilder::buildAckElicitingPacket(int maxPacketSize)
{
    QuicPacket *packet = nullptr;

    //First add frames from controlQueue in Packet
    packet = addFramesFromControlQueue(packet, maxPacketSize);

    // add stream frames
    Stream *selectedStream = scheduler->selectStream(maxPacketSize - getPacketSize(packet));

    //check if controlQueue has got ControlFrames after selectStream due to FlowControl
    packet = addFramesFromControlQueue(packet, maxPacketSize);

    // if remainingPacketSize is too small or there is no stream queue with data, selectStream should return 0.
    while (selectedStream != nullptr) {
        // is enough space remaining for a stream frame header with at least one byte of data?
        //if (remainingPacketSize < selectedStream->getHeaderSize() + streamDataSize) {
        //    return pkt;
        //}

        QuicFrame *streamFrame = selectedStream->generateNextStreamFrame(maxPacketSize - getPacketSize(packet));
        EV_DEBUG << "insert stream frame " << streamFrame << endl;

        packet = addFrameToPacket(packet, streamFrame);

        selectedStream = scheduler->selectStream(maxPacketSize - getPacketSize(packet));

        //check if controlQueue has got ControlFrames after selectStream due to FlowControl
        packet = addFramesFromControlQueue(packet, maxPacketSize);
    }

    ASSERT(packet == nullptr || (packet->getSize() <= maxPacketSize && packet->isAckEliciting()));
    return packet;
}

QuicPacket *PacketBuilder::buildAckElicitingPacket(std::vector<QuicPacket*> *sentPackets, int maxPacketSize, bool skipPacketNumber)
{
    QuicPacket *packet = nullptr;
    for (QuicPacket *sentPacket : *sentPackets) {
        for (QuicFrame *frame : *sentPacket->getFrames()) {
            if (frame->getType() == FRAME_HEADER_TYPE_STREAM) {
                // TODO: check if stream is still open
                QuicStreamFrame *streamFrame = (QuicStreamFrame *) frame;
                uint64_t offset = streamFrame->getStreamHeader()->getOffset();
                uint64_t length = streamFrame->getStreamHeader()->getLength();
                if ((getPacketSize(packet) + frame->getSize()) > maxPacketSize) {
                    length -= (getPacketSize(packet) + frame->getSize()) - maxPacketSize;
                }
                EV_DEBUG << "try to retransmit offset=" << offset << ", length=" << length << " in probe packet" << endl;
                QuicFrame *newStreamFrame = streamFrame->getStream()->generateStreamFrame(offset, length);
                if (newStreamFrame == nullptr) {
                    EV_DEBUG << "skip retransmitting, data already acked" << endl;
                } else {
                    packet = addFrameToPacket(packet, newStreamFrame, skipPacketNumber);
                    if ((maxPacketSize - getPacketSize(packet) <= StreamFrameHeader::MAX_HEADER_SIZE)) {
                        break;
                    }
                }
            }
        }
        if ((maxPacketSize - getPacketSize(packet) <= StreamFrameHeader::MAX_HEADER_SIZE)) {
            break;
        }
    }
    ASSERT(packet == nullptr || (packet->getSize() <= maxPacketSize && packet->isAckEliciting()));
    return packet;
}

QuicPacket *PacketBuilder::buildZeroRttPacket(int maxPacketSize)
{
    QuicPacket *packet = createZeroRttPacket();

    // select stream for STREAM frame
    Stream *selectedStream = nullptr;
    selectedStream = scheduler->selectStream(maxPacketSize - getPacketSize(packet));
    if (selectedStream == nullptr) {
        packetNumber[PacketNumberSpace::ApplicationData]--;
        delete packet;
        packet = nullptr;
    }
    while (selectedStream != nullptr) {
        // is enough space remaining for a stream frame header with at least one byte of data?
        //if (remainingPacketSize < selectedStream->getHeaderSize() + streamDataSize) {
        //    return pkt;
        //}

        packet = addFrameToPacket(packet, selectedStream->generateNextStreamFrame(maxPacketSize - getPacketSize(packet)));

        selectedStream = scheduler->selectStream(maxPacketSize - getPacketSize(packet));
    }

    ASSERT(packet == nullptr || packet->getSize() <= maxPacketSize);
    return packet;

}

QuicPacket *PacketBuilder::buildPingPacket()
{
    QuicPacket *packet = createOneRttPacket();
    addFrameToPacket(packet, createPingFrame());
    return packet;
}

QuicPacket *PacketBuilder::buildDplpmtudProbePacket(int packetSize, Dplpmtud *dplpmtud)
{
    if (this->skipPacketNumberForDplpmtudProbePackets) {
        // skip a packet number
        packetNumber[PacketNumberSpace::ApplicationData]++;
    }
    std::stringstream packetName;
    packetName << "DplpmtudProbePacket:" << packetNumber[PacketNumberSpace::ApplicationData];
    QuicPacket *packet = new DplpmtudProbePacket(packetName.str(), dplpmtud);
    // set short header
    packet->setHeader(createOneRttHeader());

    packet->addFrame(createPingFrame());

    int remainingBytes = packetSize - packet->getSize();
    ASSERT(remainingBytes >= 0);

    /*
    for (int i=0; i<remainingBytes; i++) {
        packet->addFrame(createPaddingFrame());
    }
    */
    packet->addFrame(createPaddingFrame(remainingBytes));

    return packet;
}

QuicPacket *PacketBuilder::buildClientInitialPacket(int maxPacketSize, TransportParameters *tp, uint32_t token)
{
    QuicPacket *packet = createPacket(PacketNumberSpace::Initial, false);
    if (token > 0) {
        Ptr<InitialPacketHeader> initialPacketHeader = staticPtrCast<InitialPacketHeader>(packet->getHeader());
        initialPacketHeader->setTokenLength(4);
        initialPacketHeader->setToken(token);
    }

    packet->addFrame(createCryptoFrame(tp));
    packet->addFrame(createPaddingFrame(1200 - packet->getSize()));

    return packet;
}

QuicPacket *PacketBuilder::buildServerInitialPacket(int maxPacketSize)
{
    QuicPacket *packet = createPacket(PacketNumberSpace::Initial, false);

    packet->addFrame(createCryptoFrame());

    // check if we would like to bundle an ack frame
    if (receivedPacketsAccountant[PacketNumberSpace::Initial]->hasNewAckInfoAboutAckElicitings() || (receivedPacketsAccountant[PacketNumberSpace::Initial]->hasNewAckInfo() && bundleAckForNonAckElicitingPackets)) {
        QuicFrame *ackFrame = receivedPacketsAccountant[PacketNumberSpace::Initial]->generateAckFrame(maxPacketSize - getPacketSize(packet));
        if (ackFrame != nullptr) {
            packet->addFrame(ackFrame);
        }
    }

    return packet;
}

QuicPacket *PacketBuilder::buildHandshakePacket(int maxPacketSize, TransportParameters *tp)
{
    QuicPacket *packet = createPacket(PacketNumberSpace::Handshake, false);

    packet->addFrame(createCryptoFrame(tp));

    // check if we would like to bundle an ack frame
    if (receivedPacketsAccountant[PacketNumberSpace::Handshake]->hasNewAckInfoAboutAckElicitings() || (receivedPacketsAccountant[PacketNumberSpace::Handshake]->hasNewAckInfo() && bundleAckForNonAckElicitingPackets)) {
        QuicFrame *ackFrame = receivedPacketsAccountant[PacketNumberSpace::Handshake]->generateAckFrame(maxPacketSize - getPacketSize(packet));
        if (ackFrame != nullptr) {
            packet->addFrame(ackFrame);
        }
    }

    return packet;
}

void PacketBuilder::addHandshakeDone()
{
    controlQueue->push_back(createHandshakeDoneFrame());
}

void PacketBuilder::addNewTokenFrame(uint32_t token)
{
    controlQueue->push_back(createNewTokenFrame(token));
}

QuicPacket *PacketBuilder::buildConnectionClosePacket(int maxPacketSize, bool sendAck, bool appInitiated, int errorCode)
{
    PacketNumberSpace pnSpace = PacketNumberSpace::ApplicationData;
    QuicPacket *packet = createPacket(pnSpace, false);

    // check if we can bundle an ack frame
    if (sendAck && receivedPacketsAccountant[pnSpace]->hasNewAckInfo()) {
        QuicFrame *ackFrame = receivedPacketsAccountant[pnSpace]->generateAckFrame(maxPacketSize - getPacketSize(packet));
        if (ackFrame != nullptr) {
            packet->addFrame(ackFrame);
        }
    }

    packet->addFrame(createConnectionCloseFrame(appInitiated, errorCode));

    return packet;

}

} /* namespace quic */
} /* namespace inet */
