//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "EstablishedConnectionState.h"
#include "CloseConnectionState.h"
#include "DrainConnectionState.h"

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

ConnectionState *EstablishedConnectionState::processCloseAppCommand(cMessage *msg)
{
    EV_DEBUG << "processCloseAppCommand in " << name << endl;
    context->close(true, true);
    return new CloseConnectionState(context);
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

    if (gotConnectionClose) {
        context->close(false, false);
        return new DrainConnectionState(context);
    }
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
    EV_DEBUG << "processAckFrame in " << name << endl;
    if (processingZeroRttPacket) {
        throw cRuntimeError("EstablishedConnectionState::processAckFrame: Processing a 0-RTT packet, which must not contain an ACK frame.");
    }
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

void EstablishedConnectionState::processCryptoFrame(const Ptr<const CryptoFrameHeader>& frameHeader, Packet *pkt)
{
    EV_DEBUG << "CryptoFrameHeader in " << name << endl;
    if (processingZeroRttPacket) {
        throw cRuntimeError("EstablishedConnectionState::processCryptoFrame: Processing a 0-RTT packet, which must not contain a CRYPTO frame.");
    }
    gotCryptoFin = true;
}

void EstablishedConnectionState::processHandshakeDoneFrame()
{
    EV_DEBUG << "HandshakeDoneFrameHeader in " << name << endl;
    if (processingZeroRttPacket) {
        throw cRuntimeError("EstablishedConnectionState::processHandshakeDoneFrame: Processing a 0-RTT packet, which must not contain a HANDSHAKE_DONE frame.");
    }
    context->setHandshakeConfirmed(true);
}

void EstablishedConnectionState::processNewTokenFrame(const Ptr<const NewTokenFrameHeader>& frameHeader)
{
    EV_DEBUG << "NewTokenFrameHeader in " << name << endl;
    if (processingZeroRttPacket) {
        throw cRuntimeError("EstablishedConnectionState::processNewTokenFrame: Processing a 0-RTT packet, which must not contain a NEW_TOKEN frame.");
    }
    context->buildClientTokenAndSendToApp(frameHeader->getToken());
}

void EstablishedConnectionState::processConnectionCloseFrame()
{
    EV_DEBUG << "processConnectionCloseFrame in " << name << endl;
    gotConnectionClose = true;
}

ConnectionState *EstablishedConnectionState::processIcmpPtb(uint64_t droppedPacketNumber, int ptbMtu)
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

    ackElicitingPacket = false;
    processFrames(pkt, PacketNumberSpace::Initial);

    context->accountReceivedPacket(packetHeader->getPacketNumber(), ackElicitingPacket, PacketNumberSpace::Initial, false);

    return this;
}

ConnectionState *EstablishedConnectionState::processHandshakePacket(const Ptr<const HandshakePacketHeader>& packetHeader, Packet *pkt) {
    EV_DEBUG << "processHandshakePacket in " << name << endl;

    ackElicitingPacket = false;
    processFrames(pkt, PacketNumberSpace::Handshake);

    context->accountReceivedPacket(packetHeader->getPacketNumber(), ackElicitingPacket, PacketNumberSpace::Handshake, false);
    context->sendAck(PacketNumberSpace::Handshake);
    if (gotCryptoFin) {
        gotCryptoFin = false;

        // stop accepting early data
        context->removeConnectionForInitialConnectionId();

        // enqueue a 0-RTT Token Frame for sending if the parameter is true
        context->enqueueZeroRttTokenFrame();

        context->setHandshakeConfirmed(true);
        context->sendHandshakeDone();
    }

    return this;
}

ConnectionState *EstablishedConnectionState::processZeroRttPacket(const Ptr<const ZeroRttPacketHeader>& packetHeader, Packet *pkt)
{
    EV_DEBUG << "processZeroRttPacket in " << name << endl;

    processingZeroRttPacket = true;

    ackElicitingPacket = false;
    processFrames(pkt, PacketNumberSpace::ApplicationData);

    context->accountReceivedPacket(packetHeader->getPacketNumber(), ackElicitingPacket, PacketNumberSpace::ApplicationData, false);

    if (gotConnectionClose) {
        context->close(false, false);
        return new DrainConnectionState(context);
    }

    processingZeroRttPacket = false;

    return this;
}

} /* namespace quic */
} /* namespace inet */
