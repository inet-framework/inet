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

#include "ConnectionState.h"
#include "../packet/PacketHeader_m.h"
#include "../packet/FrameHeader_m.h"

namespace inet {
namespace quic {

ConnectionState::ConnectionState(Connection *context) {
    this->context = context;
}

ConnectionState::~ConnectionState() {

}

void ConnectionState::start()
{

}

ConnectionState *ConnectionState::processAppCommand(cMessage *msg)
{
    switch (msg->getKind()) {
        case QUIC_C_OPEN_ACTIVE: // connect
            return processConnectAppCommand(msg);
        case QUIC_C_SEND: // send
            return processSendAppCommand(msg);
        case QUIC_C_RECEIVE:
            return processRecvAppCommand(msg);
        case QUIC_C_CLOSE: // close
            return processCloseAppCommand(msg);
        case QUIC_C_CREATE_PCB: // bind
            // socket already bound, nothing to do
            return this;
        default: // all other commands are handled in UdpSocket
            throw cRuntimeError("Unexpected/unknown app command");
    }

    return nullptr;
}

ConnectionState *ConnectionState::processConnectAppCommand(cMessage *msg)
{
    throw cRuntimeError("Connect app command unexpected in the current state");
}

ConnectionState *ConnectionState::processSendAppCommand(cMessage *msg)
{
    throw cRuntimeError("Send app command unexpected in the current state");
}

ConnectionState *ConnectionState::processRecvAppCommand(cMessage *msg)
{
    throw cRuntimeError("Send data to app command unexpected in the current state");
}

ConnectionState *ConnectionState::processCloseAppCommand(cMessage *msg)
{
    throw cRuntimeError("Close app command unexpected in the current state");
}

ConnectionState *ConnectionState::processPacket(Packet *pkt)
{
    auto packetHeader = pkt->popAtFront<PacketHeader>();
    EV_DEBUG << "process packet, found chunk: " << packetHeader << endl;

    switch (packetHeader->getHeaderForm()) {
        case PACKET_HEADER_FORM_LONG:
            switch (staticPtrCast<const LongPacketHeader>(packetHeader)->getLongPacketType()) {
                case LONG_PACKET_HEADER_TYPE_INITIAL:
                    return processInitialPacket(staticPtrCast<const InitialPacketHeader>(packetHeader), pkt);
                case LONG_PACKET_HEADER_TYPE_HANDSHAKE:
                    return processHandshakePacket(staticPtrCast<const HandshakePacketHeader>(packetHeader), pkt);
            }
            break;
        case PACKET_HEADER_FORM_SHORT:
            return processOneRttPacket(staticPtrCast<const OneRttPacketHeader>(packetHeader), pkt);
    }

    throw cRuntimeError("Unknown Packet Header Type");
}

ConnectionState *ConnectionState::processInitialPacket(const Ptr<const InitialPacketHeader>& packetHeader, Packet *pkt)
{
    throw cRuntimeError("Initial packet unexpected in the current state");
}

ConnectionState *ConnectionState::processHandshakePacket(const Ptr<const HandshakePacketHeader>& packetHeader, Packet *pkt)
{
    throw cRuntimeError("Handshake packet unexpected in the current state");
}

ConnectionState *ConnectionState::processOneRttPacket(const Ptr<const OneRttPacketHeader>& packetHeader, Packet *pkt)
{
    throw cRuntimeError("1-RTT packet unexpected in the current state");
}

ConnectionState *ConnectionState::processIcmpPtb(Packet *droppedPkt, int ptbMtu)
{
    auto packetHeader = droppedPkt->popAtFront<PacketHeader>();
    EV_DEBUG << "process ICMP PTB, found chunk: " << packetHeader << endl;

    uint64_t packetNumber = 0;
    switch (packetHeader->getHeaderForm()) {
        case PACKET_HEADER_FORM_SHORT: {
            Ptr<const ShortPacketHeader> shortPacketHeader = staticPtrCast<const ShortPacketHeader>(packetHeader);
            packetNumber = shortPacketHeader->getPacketNumber();
            break;
        }
        default: {
            throw cRuntimeError("Unexpected type of dropped packet in processIcmpPtb");
        }
    }

    return processIcmpPtb(packetNumber, ptbMtu);
}

void ConnectionState::processFrames(Packet *pkt, PacketNumberSpace pnSpace)
{
    Ptr<const FrameHeader> frameHeader = nullptr;
    do {
        processFrame(pkt, pnSpace);
        frameHeader = nullptr;
        if (pkt->getByteLength() > 0) {
            frameHeader = staticPtrCast<const FrameHeader>(pkt->peekAtFront<Chunk>());
        }
    } while (frameHeader != nullptr);
}

bool ConnectionState::containsFrame(Packet *pkt, enum FrameHeaderType frameType)
{
    Ptr<const FrameHeader> frameHeader = nullptr;
    b offset = b(0);

    //EV_DEBUG << "contains " << pkt << " a frame of type " << frameType << "?" << endl;
    do {
        if (offset >= pkt->getDataLength()) {
            //EV_DEBUG << "no, offset is " << offset << " and dataLength only " << pkt->getDataLength() << endl;
            return false;
        }
        frameHeader = staticPtrCast<const FrameHeader>(pkt->peekDataAt<Chunk>(offset));
        //EV_DEBUG << "frame at offset " << offset << " is " << frameHeader << endl;
        if (frameHeader == nullptr) {
            //EV_DEBUG << "no, frameHeader is a nullptr" << endl;
            return false;
        }
        offset += frameHeader->getChunkLength();
    } while (frameHeader->getFrameType() != frameType);

    //EV_DEBUG << "yes" << endl;
    return true;
}

void ConnectionState::discardFrames(Packet *pkt)
{
    Ptr<const FrameHeader> frameHeader = nullptr;
    do {
        frameHeader = pkt->popAtFront<FrameHeader>();
        EV_DEBUG << "discard frame " << frameHeader << endl;
    } while (pkt->getByteLength() > 0 && frameHeader != nullptr);
}

void ConnectionState::processFrame(Packet *pkt, PacketNumberSpace pnSpace)
{
    auto frameHeader = pkt->popAtFront<FrameHeader>();
    EV_DEBUG << "process frame, found chunk: " << frameHeader << endl;

    if (frameHeader->getFrameType() != FRAME_HEADER_TYPE_ACK && frameHeader->getFrameType() != FRAME_HEADER_TYPE_PADDING) {
        ackElicitingPacket = true;
    }
    switch (frameHeader->getFrameType()) {
        case FRAME_HEADER_TYPE_STREAM:
            return processStreamFrame(staticPtrCast<const StreamFrameHeader>(frameHeader), pkt);
        case FRAME_HEADER_TYPE_ACK:
            return processAckFrame(staticPtrCast<const AckFrameHeader>(frameHeader), pnSpace);
        case FRAME_HEADER_TYPE_MAX_DATA:
            return processMaxDataFrame(staticPtrCast<const MaxDataFrameHeader>(frameHeader));
        case FRAME_HEADER_TYPE_MAX_STREAM_DATA:
            return processMaxStreamDataFrame(staticPtrCast<const MaxStreamDataFrameHeader>(frameHeader));
        case FRAME_HEADER_TYPE_DATA_BLOCKED:
            return processDataBlockedFrame(staticPtrCast<const DataBlockedFrameHeader>(frameHeader));
        case FRAME_HEADER_TYPE_STREAM_DATA_BLOCKED:
            return processStreamDataBlockedFrame(staticPtrCast<const StreamDataBlockedFrameHeader>(frameHeader));
        case FRAME_HEADER_TYPE_CRYPTO:
            return processCryptoFrame(staticPtrCast<const CryptoFrameHeader>(frameHeader), pkt);
        case FRAME_HEADER_TYPE_HANDSHAKE_DONE:
            return processHandshakeDoneFrame();
        case FRAME_HEADER_TYPE_CONNECTION_CLOSE_APP:
        case FRAME_HEADER_TYPE_CONNECTION_CLOSE_QUIC:
            return processConnectionCloseFrame();
        case FRAME_HEADER_TYPE_PADDING:
        case FRAME_HEADER_TYPE_PING:
            return;
        default:
            throw cRuntimeError("Unknown Frame Header Type");
    }

}

void ConnectionState::processStreamFrame(const Ptr<const StreamFrameHeader>& frameHeader, Packet *pkt)
{
    throw cRuntimeError("Stream frame unexpected in the current state");
}

void ConnectionState::processAckFrame(const Ptr<const AckFrameHeader>& frameHeader, PacketNumberSpace pnSpace)
{
    throw cRuntimeError("Ack frame unexpected in the current state");
}

void ConnectionState::processMaxDataFrame(const Ptr<const MaxDataFrameHeader>& frameHeader)
{
    throw cRuntimeError("Max_Data frame unexpected in the current state");
}

void ConnectionState::processMaxStreamDataFrame(const Ptr<const MaxStreamDataFrameHeader>& frameHeader)
{
    throw cRuntimeError("Max_Stream_Data frame unexpected in the current state");
}

void ConnectionState::processStreamDataBlockedFrame(const Ptr<const StreamDataBlockedFrameHeader>& frameHeader)
{
    throw cRuntimeError("Stream_Data_Blocked frame unexpected in the current state");
}

void ConnectionState::processDataBlockedFrame(const Ptr<const DataBlockedFrameHeader>& frameHeader)
{
    throw cRuntimeError("Data_Blocked frame unexpected in the current state");
}

void ConnectionState::processCryptoFrame(const Ptr<const CryptoFrameHeader>& frameHeader, Packet *pkt)
{
    throw cRuntimeError("CRYPTO frame unexpected in the current state");
}

void ConnectionState::processHandshakeDoneFrame()
{
    throw cRuntimeError("HANDSHAKE_DONE frame unexpected in the current state");
}

void ConnectionState::processConnectionCloseFrame()
{
    throw cRuntimeError("CONNECTION_CLOSE frame unexpected in the current state");
}

ConnectionState *ConnectionState::processIcmpPtb(uint64_t droppedPacketNumber, int ptbMtu)
{
    EV_WARN << "PTB not handled in current state" << this << endl;
    return this;
}

ConnectionState *ConnectionState::processTimeout(cMessage *msg)
{
    switch (msg->getKind()) {
        case LOSS_DETECTION_TIMER:
            return processLossDetectionTimeout(msg);
        case ACK_DELAY_TIMER:
            return processAckDelayTimeout(msg);
        case DPLPMTUD_RAISE_TIMER:
            return processDplpmtudRaiseTimeout(msg);
    }

    throw cRuntimeError("Unknown Timer Type");
}

ConnectionState *ConnectionState::processLossDetectionTimeout(cMessage *msg)
{
    throw cRuntimeError("Loss Detection Timeout unexpected in the current state");
}

ConnectionState *ConnectionState::processAckDelayTimeout(cMessage *msg)
{
    throw cRuntimeError("Ack Delay Timeout unexpected in the current state");
}

ConnectionState *ConnectionState::processDplpmtudRaiseTimeout(cMessage *msg)
{
    throw cRuntimeError("DPLPMTUD Raise Timeout unexpected in the current state");
}

} /* namespace quic */
} /* namespace inet */
