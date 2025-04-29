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

#include "PacketBuilder.h"
#include "packet/PacketHeader_m.h"
#include "inet/common/Units.h"
#include "packet/QuicStreamFrame.h"

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
    if (dstConnectionId == nullptr) {
        packetHeader->setDstConnectionIdLength(8);
        packetHeader->setDstConnectionId(0x1122334455667788);
    } else {
        packetHeader->setDstConnectionIdLength(dstConnectionId->getLength());
        packetHeader->setDstConnectionId(dstConnectionId->getId());
    }
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

Ptr<ShortPacketHeader> PacketBuilder::createOneRttHeader()
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

QuicPacket *PacketBuilder::createPacket(PacketNumberSpace pnSpace, bool skipPacketNumber)
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
            packetName << "1-RTT[" << packetNumber[pnSpace] << "]";
            header = createOneRttHeader();
            break;
    }
    QuicPacket *packet = new QuicPacket(packetName.str());;
    packet->setHeader(header);

    return packet;
}

QuicPacket *PacketBuilder::createOneRttPacket(bool skipPacketNumber)
{
    return createPacket(PacketNumberSpace::ApplicationData, skipPacketNumber);
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
    frameHeader->setOffset(0);
    frameHeader->setLength(0);
    frameHeader->calcChunkLength();
    QuicFrame *frame = new QuicFrame(frameHeader);

    if (tp) {
        frameHeader->setContainsTransportParameters(true);
        Ptr<TransportParametersExtension> transportParametersExt = makeShared<TransportParametersExtension>();
        transportParametersExt->setInitialMaxData(tp->initialMaxData);
        transportParametersExt->setInitialMaxStreamDataBidiLocal(tp->initialMaxStreamData);
        transportParametersExt->setInitialMaxStreamDataBidiRemote(tp->initialMaxStreamData);
        transportParametersExt->setInitialMaxStreamDataUni(tp->initialMaxStreamData);
        transportParametersExt->calcChunkLength();
        frame->setData(transportParametersExt);
    }
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

/**
 * Builds a regular short header packet, that may contain
 * (1) an ack frame,
 * (2) control frames, and/or
 * (3) stream frames.
 *
 * \param maxPacketSize Upper limit for the size of the packet that will be created.
 * \return Pointer to the created QuicHeader object.
 * TODO: ensure that this method do not create a packet without a frame.
 */
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

/**
 * Builds a packet that is ack eliciting. That means, for now, a packet that include control and/or stream frames.
 * \param maxPacketSize Specifies an upper limit for the packet created by this method.
 * \return A pointer to a ack eliciting packet with new stream data, or nullptr, if no new stream data were available.
 */
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

/**
 * Builds a packet that is ack eliciting by resending data from STREAM frames of outstanding packets.
 * \param sentPackets Outstanding packets that should contain a stream frame for the new packet.
 * \param maxPacketSize Upper limit for the size of the new packet.
 * \return A pointer to an ack elicting packet with stream data from the given outstanding packets,
 * or nullptr, if there is no retransmitable stream frame in the outstanding packet.
 */
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

QuicPacket *PacketBuilder::buildClientInitialPacket(int maxPacketSize, TransportParameters *tp)
{
    QuicPacket *packet = createPacket(PacketNumberSpace::Initial, false);

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
