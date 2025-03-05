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

#include "EstablishedConnectionState.h"

namespace inet {
namespace quic {

void EstablishedConnectionState::start()
{
    context->established();
}

ConnectionState *EstablishedConnectionState::processSendAppCommand(cMessage *msg)
{
    Packet *pkt = check_and_cast<Packet *>(msg);
    auto streamId = pkt->getTag<QuicStreamReq>()->getStreamID();
    context->newStreamData(streamId, pkt->peekData());
    //delete pkt;

    return this;
}

ConnectionState *EstablishedConnectionState::processRecvAppCommand(cMessage *msg)
{
    QuicRecvCommand *rcvCommand = dynamic_cast<QuicRecvCommand *>(msg->getControlInfo());
    context->sendDataToApp(rcvCommand->getStreamID(), B(rcvCommand->getExpectedDataSize()));
    //delete msg;

    return this;
}

ConnectionState *EstablishedConnectionState::processOneRttPacket(const Ptr<const OneRttPacketHeader>& packetHeader, Packet *pkt)
{
    EV_DEBUG << "processOneRttPacket in " << name << endl;

    ackElicitingPacket = false;
    processFrames(pkt, PacketNumberSpace::ApplicationData);

    context->accountReceivedPacket(packetHeader->getPacketNumber(), ackElicitingPacket, PacketNumberSpace::ApplicationData, packetHeader->getIBit());

    return this;
}

void EstablishedConnectionState::processStreamFrame(const Ptr<const StreamFrameHeader>& frameHeader, Packet *pkt)
{
    EV_DEBUG << "processStreamFrame in " << name << endl;

    // TODO: Check if data is following (it might be an empty Stream Frame)
    auto data = pkt->popAtFront();
    EV_DEBUG << "process data, found chunk: " << data << endl;

    context->processReceivedData(frameHeader->getStreamId(), frameHeader->getOffset(), data);
}

void EstablishedConnectionState::processAckFrame(const Ptr<const AckFrameHeader>& frameHeader, PacketNumberSpace pnSpace)
{
    context->handleAckFrame(frameHeader, pnSpace);
}

void EstablishedConnectionState::processMaxDataFrame(const Ptr<const MaxDataFrameHeader>& frameHeader){
     EV_DEBUG << "processMaxDataFrame in " << name << endl;
     context->onMaxDataFrameReceived(frameHeader->getMaximumData());
}

void EstablishedConnectionState::processMaxStreamDataFrame(const Ptr<const MaxStreamDataFrameHeader>& frameHeader)
{
    EV_DEBUG << "processMaxStreamDataFrame in " << name << endl;
    context->onMaxStreamDataFrameReceived(frameHeader->getStreamId(), frameHeader->getMaximumStreamData());
}

void EstablishedConnectionState::processStreamDataBlockedFrame(const Ptr<const StreamDataBlockedFrameHeader>& frameHeader)
{
    EV_DEBUG << "StreamDataBlockedFrame in " << name << endl;
    context->onStreamDataBlockedFrameReceived(frameHeader->getStreamId(), frameHeader->getStreamDataLimit());
}

void EstablishedConnectionState::processDataBlockedFrame(const Ptr<const DataBlockedFrameHeader>& frameHeader)
{
    EV_DEBUG << "DataBlockedFrameHeader in " << name << endl;
    context->onDataBlockedFrameReceived(frameHeader->getDataLimit());
}

ConnectionState *EstablishedConnectionState::processIcmpPtb(uint32_t droppedPacketNumber, int ptbMtu)
{
    context->reportPtb(droppedPacketNumber, ptbMtu);
    return this;
}

ConnectionState *EstablishedConnectionState::processLossDetectionTimeout(cMessage *msg)
{
    context->getReliabilityManager()->onLossDetectionTimeout();

    // if packets have detected lost, we might be able to send new packets
    context->sendPackets();

    return this;
}

ConnectionState *EstablishedConnectionState::processAckDelayTimeout(cMessage *msg)
{
    context->getReceivedPacketsAccountant(PacketNumberSpace::ApplicationData)->onAckDelayTimeout();
    if (context->getReceivedPacketsAccountant(PacketNumberSpace::ApplicationData)->wantsToSendAckImmediately()) {
        context->sendPackets();
    }

    return this;
}

ConnectionState *EstablishedConnectionState::processDplpmtudRaiseTimeout(cMessage *msg)
{
    context->getPath()->getDplpmtud()->onRaiseTimeout();
    return this;
}

ConnectionState *EstablishedConnectionState::processInitialPacket(const Ptr<const InitialPacketHeader>& packetHeader, Packet *pkt) {
    EV_DEBUG << "processInitialPacket in " << name << endl;

    processFrames(pkt, PacketNumberSpace::Initial);

    context->accountReceivedPacket(packetHeader->getPacketNumber(), ackElicitingPacket, PacketNumberSpace::Initial, false);

    return this;
}

ConnectionState *EstablishedConnectionState::processHandshakePacket(const Ptr<const HandshakePacketHeader>& packetHeader, Packet *pkt) {
    EV_DEBUG << "processHandshakePacket in " << name << endl;

    processFrames(pkt, PacketNumberSpace::Handshake);

    context->accountReceivedPacket(packetHeader->getPacketNumber(), ackElicitingPacket, PacketNumberSpace::Handshake, false);
    context->sendAck(PacketNumberSpace::Handshake);

    return this;
}


} /* namespace quic */
} /* namespace inet */
