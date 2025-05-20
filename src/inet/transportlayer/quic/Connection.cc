//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "Connection.h"
#include "inet/transportlayer/contract/quic/QuicCommand_m.h"
#include "packet/PacketHeader_m.h"
#include "packet/FrameHeader_m.h"
#include "inet/common/socket/SocketTag_m.h"
#include "AppSocket.h"
#include "connectionstate/ConnectionState.h"
#include "connectionstate/EstablishedConnectionState.h"
#include "connectionstate/InitialConnectionState.h"
#include "PacketBuilder.h"
#include "scheduler/RRScheduler.h"
#include "congestioncontrol/CongestionControlFactory.h"
#include "PmtuValidator.h"
#include "exception/InvalidTokenException.h"

namespace inet {
namespace quic {

Connection::Connection(Quic *quicSimpleMod, UdpSocket *udpSocket, AppSocket *appSocket, L3Address remoteAddr, uint16_t remotePort, uint64_t srcConnectionId) {
    this->quicSimpleMod = quicSimpleMod;
    this->udpSocket = udpSocket;
    this->appSocket = appSocket;

    bool useDplpmtud = quicSimpleMod->par("useDplpmtud");

    srcConnectionIds.push_back(new ConnectionId(srcConnectionId));

    stats = new Statistics(quicSimpleMod, "_cid=" + std::to_string(srcConnectionIds[0]->getId()));

    lastMaxQuicPacketSize = 0;
    usedMaxQuicPacketSizeStat = stats->createStatisticEntry("usedMaxQuicPacketSize");
    packetNumberReceivedStat = stats->createStatisticEntry("packetNumberReceived");
    packetNumberSentStat = stats->createStatisticEntry("packetNumberSent");

    this->path = new Path(this, udpSocket->getLocalAddr(), udpSocket->getLocalPort(), remoteAddr, remotePort, useDplpmtud, stats);
    dplpmutdInIntialBase = false;
    if (path->usesDplpmtud()) {
        dplpmutdInIntialBase = true;
    }

    this->connectionState = new InitialConnectionState(this);

    localTransportParameters = new TransportParameters();
    localTransportParameters->readParameters(quicSimpleMod);
    remoteTransportParameters = new TransportParameters();

    acceptDataFromApp = true;
    sendQueueLimit = quicSimpleMod->par("sendQueueLimit");
    sendQueueLowWaterRatio = quicSimpleMod->par("sendQueueLowWaterRatio");
    maxDataFrameThreshold = quicSimpleMod->par("maxDataFrameThreshold");
    maxStreamDataFrameThreshold = quicSimpleMod->par("maxStreamDataFrameThreshold");
    roundConsumedDataValue = quicSimpleMod->par("roundConsumedDataValue");
    sendMaxDataFramesImmediately = quicSimpleMod->par("sendMaxDataFramesImmediately");
    useCwndForParallelProbes = quicSimpleMod->par("useCwndForParallelProbes");
    sendDataDuringInitalDplpmtudBase = quicSimpleMod->par("sendDataDuringInitalDplpmtudBase");
    //reduceTlpSizeOnlyIfPmtuInvalidPossible = quicSimpleMod->par("reduceTlpSizeOnlyIfPmtuInvalidPossible");

    // TODO: create Scheduler depending on ned file parameter
    scheduler = new RRScheduler(&streamMap);

    //receivedPacketsAccountant = new ReceivedPacketsAccountant(createTimer(TimerType::ACK_DELAY_TIMER, "AckDelayTimer"), transportParameter);
    //receivedPacketsAccountant->readParameters(quicSimpleMod);

    receivedPacketsAccountants[PacketNumberSpace::ApplicationData] = new ReceivedPacketsAccountant(createTimer(TimerType::ACK_DELAY_TIMER, "AckDelayTimer"), remoteTransportParameters);
    receivedPacketsAccountants[PacketNumberSpace::ApplicationData]->readParameters(quicSimpleMod);

    receivedPacketsAccountants[PacketNumberSpace::Initial] = new ReceivedPacketsAccountant(nullptr, remoteTransportParameters);
    receivedPacketsAccountants[PacketNumberSpace::Handshake] = new ReceivedPacketsAccountant(nullptr, remoteTransportParameters);

    packetBuilder = new PacketBuilder(&controlQueue, scheduler, receivedPacketsAccountants);
    packetBuilder->setSrcConnectionId(srcConnectionIds[0]);
    packetBuilder->readParameters(quicSimpleMod);

    congestionController = CongestionControlFactory::getInstance()->createCongestionController(quicSimpleMod->par("congestionControl"));
    congestionController->readParameters(quicSimpleMod);
    congestionController->setStatistics(stats);
    congestionController->setPath(path);

    reliabilityManager = new ReliabilityManager(this, remoteTransportParameters, receivedPacketsAccountants[PacketNumberSpace::ApplicationData], congestionController, stats);

    connectionFlowController = new ConnectionFlowController(remoteTransportParameters->initialMaxData, stats);
    connectionFlowControlResponder = new ConnectionFlowControlResponder(this, localTransportParameters->initialMaxData, maxDataFrameThreshold, roundConsumedDataValue, stats);
}

Connection::~Connection() {
    if (!closed) {
        delete reliabilityManager;
        delete receivedPacketsAccountants[PacketNumberSpace::ApplicationData];
        delete receivedPacketsAccountants[PacketNumberSpace::Initial];
        delete receivedPacketsAccountants[PacketNumberSpace::Handshake];
        delete congestionController;
        delete connectionFlowController;
        delete connectionFlowControlResponder;
        delete localTransportParameters;
        delete remoteTransportParameters;
        delete scheduler;

        for (QuicFrame *frame : controlQueue) {
            delete frame;
        }
        // delete stream objects
        for (auto it = streamMap.begin(); it != streamMap.end(); it++) {
            delete it->second;
        }

    }
    delete path;
    delete stats;
    delete connectionState;
    delete packetBuilder;
    if (closeTimer != nullptr) {
        delete closeTimer;
    }
    for (ConnectionId *connectionId : dstConnectionIds) {
        delete connectionId;
    }
    for (ConnectionId *connectionId : srcConnectionIds) {
        delete connectionId;
    }
}

void Connection::processAppCommand(cMessage *msg)
{
    ConnectionState *newState = connectionState->processAppCommand(msg);
    if (newState != connectionState) {
        // state transition
        delete connectionState;
        connectionState = newState;
        connectionState->start();
    }
}

uint64_t Connection::getStreamsSendQueueLength()
{
    uint64_t length = 0;
    for (auto it = streamMap.begin(); it != streamMap.end(); it++) {
        length += it->second->getSendQueueLength();
    }
    return length;
}

void Connection::newStreamData(uint64_t streamId, Ptr<const Chunk> data)
{
    if (acceptDataFromApp) {
        Stream *stream = findOrCreateStream(streamId);
        stream->enqueueDataFromApp(data);
        sendPackets();

        if (sendQueueLimit <= getStreamsSendQueueLength()) {
            // tell app that it must stop sending
            acceptDataFromApp = false;
            appSocket->sendSendQueueFull();
        }
    } else {
        // tell app that we are not able to accept any more data
        appSocket->sendMsgRejected();
    }
}

Stream *Connection::findOrCreateStream(uint64_t streamId)
{
    auto it = streamMap.find(streamId);
    // return appSocket, if found
    if (it != streamMap.end()) {
        return it->second;
    }
    // create new Stream
    Stream *stream = new Stream(streamId, this, stats);
    streamMap.insert({ streamId, stream });
    return stream;
}

void Connection::processPackets(Packet *pkt)
{
    EV_TRACE << "enter Connection::processPackets" << endl;
    stats->getMod()->emit(packetReceivedSignal, pkt);

    // process each QUIC packet in udp datagram
    while (pkt->getByteLength() > 0) {
        EV_DEBUG << "pkt->getCreationTime(): " << pkt->getCreationTime() << ", pkt->getSendingTime(): " << pkt->getSendingTime() << endl;
        int64_t byteLengthBefore = pkt->getByteLength();
        ConnectionState *newState = connectionState->processPacket(pkt);
        if (newState != connectionState) {
            // state transition
            delete connectionState;
            connectionState = newState;
            connectionState->start();
        }
        if (pkt->getByteLength() >= byteLengthBefore) {
            throw cRuntimeError("no packet processing happened");
        }
    }
    EV_TRACE << "leave Connection::processPackets" << endl;
}

void Connection::processTimeout(cMessage *msg)
{
    ConnectionState *newState = connectionState->processTimeout(msg);
    if (newState != connectionState) {
        // state transition
        delete connectionState;
        connectionState = newState;
        connectionState->start();
    }
}

void Connection::processIcmpPtb(Packet *droppedPkt, int ptbMtu)
{
    ConnectionState *newState = connectionState->processIcmpPtb(droppedPkt, ptbMtu);
    if (newState != connectionState) {
        // state transition
        delete connectionState;
        connectionState = newState;
        connectionState->start();
    }
}

void Connection::sendPackets()
{
    if (remoteTransportParametersInitialized == false) {
        // cannot send data packets until remote transport parameters have been initialized
        EV_INFO << "Connection::sendPackets: remote transport parameters not initialized, skip sending data packets." << endl;
        return;
    }

    // send packets as long as congestion control allows sending a full sized packet
    // TODO: Implement a better way to determine app limitation
    congestionController->setAppLimited(false);
    int maxQuicPacketSize = path->getMaxQuicPacketSize();
    while (congestionController->getRemainingCongestionWindow() >= maxQuicPacketSize) {
        EV_DEBUG << "cwnd=" << congestionController->getRemainingCongestionWindow() << " is larger than maxQuicPacketSize=" << maxQuicPacketSize << " - build packet for sending" << endl;

        QuicPacket *packet = nullptr;
        Dplpmtud *dplpmtud = path->getDplpmtud();

        if (path->usesDplpmtud() && (
                dplpmtud->needToSendProbe()
                || (useCwndForParallelProbes == 2 && dplpmtud->canSendProbe(congestionController->getRemainingCongestionWindow()))
            ) && dplpmtud->getProbeSize() <= congestionController->getRemainingCongestionWindow()) {

        //if (path->usesDplpmtud() && dplpmtud->needToSendProbe() && dplpmtud->getProbeSize() <= congestionController->getRemainingCongestionWindow()) {
            // send DPLPMTUD probe packet
            packet = packetBuilder->buildDplpmtudProbePacket(dplpmtud->getProbeSize(), dplpmtud);
            dplpmtud->probePacketBuilt();
        } else if(sendDataDuringInitalDplpmtudBase || !dplpmutdInIntialBase) {
            if (dynamic_cast<EstablishedConnectionState *>(connectionState) != nullptr) {
                // Connection established, build 1-RTT packet.
                packet = packetBuilder->buildPacket(maxQuicPacketSize, path->getSafeQuicPacketSize());
            } else {
                // Connection not yet established, build 0-RTT packet.
                packet = packetBuilder->buildZeroRttPacket(maxQuicPacketSize);
            }
            if (packet != nullptr // packet was created
                    && packet->isAckEliciting() // it is ack eliciting an therefore not bound by safeQuicPacketSize
                    && maxQuicPacketSize != lastMaxQuicPacketSize) {
                stats->getMod()->emit(usedMaxQuicPacketSizeStat, maxQuicPacketSize);
                lastMaxQuicPacketSize = maxQuicPacketSize;
            }
        }
        if (packet == nullptr && useCwndForParallelProbes == 1) {
            if (path->usesDplpmtud() && dplpmtud->canSendProbe(congestionController->getRemainingCongestionWindow()) && dplpmtud->getProbeSize() <= congestionController->getRemainingCongestionWindow()) {
                // send DPLPMTUD probe packet
                packet = packetBuilder->buildDplpmtudProbePacket(dplpmtud->getProbeSize(), dplpmtud);
                dplpmtud->probePacketBuilt();
            }
        }
        if (packet == nullptr) {
            // PacketBuilder wasn't able to build a packet
            // Case 1: There is nothing to send
            // Case 2: There are no control frames to send and Flow Control prohibits sending data
            congestionController->setAppLimited(true);
            break;
        } else if ((congestionController->getRemainingCongestionWindow() - packet->getSize()) < maxQuicPacketSize) {
            // this is the last packet the current congestion window allows to send -> set I-Bit.
            packet->setIBit(true);
        }
        sendPacket(packet, PacketNumberSpace::ApplicationData); // updates cwnd
    }
    if (receivedPacketsAccountants[PacketNumberSpace::ApplicationData]->wantsToSendAckImmediately()) {
        // ReceivedPacketsAccountant still needs to send an ACK frame
        // CongestionController might prohibit sending, but we can still send an Ack-only-packet
        // restrict the size of the packet by a safe upper limit, because for a non-ack-eliciting packet we have no PMTU validation.
        QuicPacket *packet = packetBuilder->buildAckOnlyPacket(path->getSafeQuicPacketSize(), PacketNumberSpace::ApplicationData);
        sendPacket(packet, PacketNumberSpace::ApplicationData);
    }
}

void Connection::sendProbePacket(uint ptoCount)
{
    EV_DEBUG << "send probe packet, ptoCount=" << ptoCount << endl;

    //uint reduceTlpSize = quicSimpleMod->par("reduceTlpSize");
    int maxQuicPacketSize = path->getMaxQuicPacketSize();
    /*
    if (path->hasPmtuValidator() && reduceTlpSize != 0 && ptoCount >= reduceTlpSize) {
        if (reduceTlpSizeOnlyIfPmtuInvalidPossible) {
            // create a copy of PmtuValidator, ...
            PmtuValidator *pmtuValidator = new PmtuValidator(path->getPmtuValidator());
            // ... report to it all outstanding ack-eliciting packets as lost and ...
            pmtuValidator->onPacketsLost(reliabilityManager->getAckElicitingSentPackets(), false);
            // ... check if this would result in the assumption that the PMTU decreased. Reduce size of TLP only if it would.
            int largestAckedSinceFirst = 0;
            if (pmtuValidator->doLostPacketsFulfillPmtuInvalidCriterium(largestAckedSinceFirst)) {
                packetSize = path->getSafeQuicPacketSize();
            }
            delete pmtuValidator;
        } else {
            packetSize = path->getSafeQuicPacketSize();
        }
    }
    */

    // 1. create packet with new data
    QuicPacket *packet = packetBuilder->buildAckElicitingPacket(maxQuicPacketSize);

    if (packet == nullptr) {
        EV_DEBUG << "no new data available, try to retransmit data" << endl;
        // 2. retransmit unacked packet
        std::vector<QuicPacket*> *ackElicitingSentPackets = reliabilityManager->getAckElicitingSentPackets(PacketNumberSpace::ApplicationData);
        if (!ackElicitingSentPackets->empty()) {
            packet = packetBuilder->buildAckElicitingPacket(ackElicitingSentPackets, maxQuicPacketSize);
        }
        delete ackElicitingSentPackets;
    }

    if (packet == nullptr) {
        EV_DEBUG << "no retransmitable data available, send ping" << endl;
        // 3. create ping packet
        packet = packetBuilder->buildPingPacket();
    } else {
        if (maxQuicPacketSize != lastMaxQuicPacketSize) {
            stats->getMod()->emit(usedMaxQuicPacketSizeStat, maxQuicPacketSize);
            lastMaxQuicPacketSize = maxQuicPacketSize;
        }
    }

    EV_DEBUG << "sendProbePacket: send packet" << endl;
    sendPacket(packet, PacketNumberSpace::ApplicationData);
}

void Connection::sendClientInitialPacket(uint32_t token) {
    int maxQuicPacketSize = path->getMaxQuicPacketSize();
    sendPacket(packetBuilder->buildClientInitialPacket(maxQuicPacketSize, localTransportParameters, token), PacketNumberSpace::Initial);
}

void Connection::sendServerInitialPacket() {
    int maxQuicPacketSize = path->getMaxQuicPacketSize();
    sendPacket(packetBuilder->buildServerInitialPacket(maxQuicPacketSize), PacketNumberSpace::Initial);
}

void Connection::sendHandshakePacket(bool includeTransportParamters) {
    int maxQuicPacketSize = path->getMaxQuicPacketSize();

    QuicPacket *packet;
    if (includeTransportParamters) {
        packet = packetBuilder->buildHandshakePacket(maxQuicPacketSize, localTransportParameters);
    } else {
        packet = packetBuilder->buildHandshakePacket(maxQuicPacketSize, nullptr);
    }
    sendPacket(packet, PacketNumberSpace::Handshake);
}

void Connection::sendPacket(QuicPacket *packet, PacketNumberSpace pnSpace, bool track)
{
    stats->getMod()->emit(packetNumberSentStat, packet->getPacketNumber());
    Packet *pkt = packet->createOmnetPacket();
    stats->getMod()->emit(packetSentSignal, pkt);
    udpSocket->sendto(path->getRemoteAddr(), path->getRemotePort(), pkt);
    if (track) {
        reliabilityManager->onPacketSent(packet, pnSpace);
    }
}

void Connection::processReceivedData(uint64_t streamId, uint64_t offset, Ptr<const Chunk> data)
{
    Stream *stream = findOrCreateStream(streamId);
    uint64_t dataLength = B(data->getChunkLength()).get();

    static simsignal_t totalRcvAppDataStat = stats->createStatisticEntry("totalRcvAppData");
    static unsigned long totalReceivedDataBytes = 0;
    totalReceivedDataBytes += dataLength;
    stats->getMod()->emit(totalRcvAppDataStat, totalReceivedDataBytes);

    //check rwnd before process data
    if(stream->isAllowedToReceivedData(dataLength)){

        stream->bufferReceivedData(data, offset);
        stream->updateHighestRecievedOffset(offset + dataLength);
        stream->measureStreamRcvDataBytes(dataLength);

        connectionFlowControlResponder->updateHighestRecievedOffset(dataLength);
    }

    if (stream->hasDataForApp()) appSocket->sendDataNotification(streamId, stream->getAvailableDataSizeForApp());
}

void Connection::sendDataToApp(uint64_t streamId, B expectedDataSize)
{
    Stream *stream = findOrCreateStream(streamId);
    int numControlFramesBefore = controlQueue.size();
    if (stream->hasDataForApp()) appSocket->sendData(stream->getDataForApp(expectedDataSize));
    if (sendMaxDataFramesImmediately && controlQueue.size() > numControlFramesBefore) {
        // Consuming data by the app generated a MAX_(STREAM)_DATA frame, which we want to send immediately
        sendPackets();
    }
}

void Connection::accountReceivedPacket(uint64_t packetNumber, bool ackEliciting, PacketNumberSpace pnSpace, bool isIBitSet)
{
    stats->getMod()->emit(packetNumberReceivedStat, packetNumber);
    receivedPacketsAccountants[pnSpace]->onPacketReceived(packetNumber, ackEliciting, isIBitSet);
    if (pnSpace == PacketNumberSpace::ApplicationData && receivedPacketsAccountants[pnSpace]->wantsToSendAckImmediately()) {
        sendPackets();
    }
}

Timer *Connection::createTimer(TimerType kind, std::string name) {
    cMessage *msg = new cMessage(name.c_str(), kind);
    return createTimer(msg);
}

Timer *Connection::createTimer(cMessage *msg) {
    msg->setContextPointer(this);
    return new Timer(quicSimpleMod, msg);
}

void Connection::onMaxDataFrameReceived(uint64_t maxData){
    connectionFlowController->onMaxFrameReceived(maxData);
    // MAX_DATA frame might enable us to send packets, when we were previously blocked by the flow control
    sendPackets();
}

void Connection::onMaxDataFrameLost(){
    auto maxDataFrame = connectionFlowControlResponder->onMaxDataFrameLost();
    controlQueue.push_back(maxDataFrame);
    sendPackets(); // send retransmission of FC update immediately
}

void Connection::onMaxStreamDataFrameReceived(uint64_t streamId, uint64_t maxStreamData){
    Stream *stream = findOrCreateStream(streamId);
    stream->onMaxStreamDataFrameReceived(maxStreamData);
    // MAX_STREAM_DATA frame might enable us to send packets, when we were previously blocked by the flow control
    sendPackets();
}

void Connection::onStreamDataBlockedFrameReceived(uint64_t streamId, uint64_t streamDataLimit){
    Stream *stream = findOrCreateStream(streamId);
    stream->onDataBlockedFrameReceived(streamDataLimit);
}

void Connection::onDataBlockedFrameReceived(uint64_t dataLimit){
    connectionFlowControlResponder->onDataBlockedFrameReceived(dataLimit);
}

void Connection::handleAckFrame(const Ptr<const AckFrameHeader>& frameHeader, PacketNumberSpace pnSpace)
{
    reliabilityManager->onAckReceived(frameHeader, pnSpace);

    if (!acceptDataFromApp) {
        if (getStreamsSendQueueLength() < sendQueueLimit*sendQueueLowWaterRatio) {
            // tell app that it can send again
            acceptDataFromApp = true;
            appSocket->sendSendQueueDrain();
        }
    }

    // ack might opened the cwnd
    sendPackets();
}

void Connection::reportPtb(uint64_t droppedPacketNumber, int ptbMtu)
{
    QuicPacket *packet = reliabilityManager->getSentPacket(PacketNumberSpace::ApplicationData, droppedPacketNumber);
    if (packet == nullptr) {
        EV_WARN << "could not find sent packet that matches to the packet reflected inside the ICMP packet" << endl;
        return;
    }

    if (path->usesDplpmtud()) {
        path->getDplpmtud()->onPtbReceived(packet->getSize(), ptbMtu);
    }
}

void Connection::setHandshakeConfirmed(bool value)
{
    handshakeConfirmed = value;
}

bool Connection::isHandshakeConfirmed()
{
    return handshakeConfirmed;
}


void Connection::addDstConnectionId(uint64_t id, uint8_t length)
{
    ConnectionId *connectionId = new ConnectionId(id, length);
    dstConnectionIds.push_back(connectionId);
    if (dstConnectionIds.size() == 1) {
        packetBuilder->setDstConnectionId(connectionId);
    }
}

void Connection::sendAck(PacketNumberSpace pnSpace)
{
    if (receivedPacketsAccountants[pnSpace]->hasNewAckInfoAboutAckElicitings()) {
        int maxQuicPacketSize = path->getMaxQuicPacketSize();
        sendPacket(packetBuilder->buildAckOnlyPacket(maxQuicPacketSize, pnSpace), pnSpace);
    }
}

void Connection::established()
{
    appSocket->sendEstablished();

    if (path->usesDplpmtud()) {
        path->getDplpmtud()->start();
    }
}

void Connection::sendHandshakeDone()
{
    packetBuilder->addHandshakeDone();
    sendPackets();
}

void Connection::close(bool sendAck, bool appInitiated)
{
    if (closed) {
        sendConnectionClose(false, appInitiated, 0);
        return;
    }
    sendConnectionClose(sendAck, appInitiated, 0);
    appSocket->sendClosed();

    // start connection close timer and delete connection on timeout
    assert(closeTimer == nullptr);
    closeTimer = createTimer(TimerType::CONNECTION_CLOSE_TIMER, "ConnectionCloseTimer");
    SimTime ptoDuration = reliabilityManager->getPtoDuration(PacketNumberSpace::ApplicationData);
    closeTimer->scheduleAt(simTime() + 3*ptoDuration);

    delete reliabilityManager;
    delete receivedPacketsAccountants[PacketNumberSpace::ApplicationData];
    delete receivedPacketsAccountants[PacketNumberSpace::Initial];
    delete receivedPacketsAccountants[PacketNumberSpace::Handshake];
    delete congestionController;
    delete connectionFlowController;
    delete connectionFlowControlResponder;
    delete localTransportParameters;
    delete remoteTransportParameters;
    delete scheduler;

    for (QuicFrame *frame : controlQueue) {
        delete frame;
    }
    // delete stream objects
    for (auto it = streamMap.begin(); it != streamMap.end(); it++) {
        delete it->second;
    }
    path->onClose();

    closed = true;
}

void Connection::sendConnectionClose(bool sendAck, bool appInitiated, int errorCode)
{
    EV_DEBUG << "sendConnectionClose" << endl;
    QuicPacket *closePacket = packetBuilder->buildConnectionClosePacket(path->getSafeQuicPacketSize(), sendAck, appInitiated, errorCode);
    sendPacket(closePacket, PacketNumberSpace::ApplicationData, false);
    // Since we send and forget this packet, we have to delete it directly.
    delete closePacket;
}

bool Connection::belongsPacketTo(Packet *pkt, uint64_t dstConnectionId)
{
    // does the list of used destination connection IDs contains the given ID?
    bool containsConnectionId = false;
    for (ConnectionId *connectionId: dstConnectionIds) {
        if (connectionId->getId() == dstConnectionId) {
            containsConnectionId = true;
            break;
        }
    }
    if (containsConnectionId == false) {
        return false;
    }

    // Does the list of sent packets contains a packet with the same packet number as the given packet?
    auto shortPacketHeader = pkt->peekAtFront<ShortPacketHeader>();
    uint64_t packetNumber = shortPacketHeader->getPacketNumber();
    QuicPacket *quicPacket = reliabilityManager->getSentPacket(PacketNumberSpace::ApplicationData, packetNumber);
    if (quicPacket == nullptr) {
        return false;
    }

    // Does the given packet has the same authentication tag as the sent packet?
    // TODO: add an authtentication tag at the end of each outgoing packet
    // and use it here to check if this given packet was sent over this connection
    return true;
}

void Connection::enqueueZeroRttTokenFrame()
{
    if (quicSimpleMod->par("sendZeroRttTokenAsServer")) {
        // generate token larger than 0. A token == 0 is used as a special value at some places
        uint32_t token;
        do {
            token = quicSimpleMod->intrand(UINT32_MAX);
        } while (token == 0);
        packetBuilder->addNewTokenFrame(token);
        udpSocket->saveToken(token, path->getRemoteAddr());
    }
}

void Connection::buildClientTokenAndSendToApp(uint32_t token)
{
    std::ostringstream clientToken;
    clientToken << token
                << "_"
                << remoteTransportParameters->initialMaxData
                << "_"
                << remoteTransportParameters->initialMaxStreamData;

    appSocket->sendToken(clientToken.str());
}

uint32_t Connection::processClientTokenExtractToken(const char *clientToken)
{
    std::stringstream clientTokenStream;
    char underscore;
    uint64_t maxData, maxStreamData;
    uint32_t token;

    clientTokenStream << clientToken;

    token = 0;
    clientTokenStream >> token;
    if (token == 0) {
        throw InvalidTokenException("Cannot read token out of given clientToken");
    }

    underscore = ' ';
    clientTokenStream >> underscore;
    if (underscore != '_') {
        throw InvalidTokenException("Invalid format of given clientToken");
    }

    maxData = 0;
    clientTokenStream >> maxData;
    if (maxData == 0) {
        throw InvalidTokenException("Cannot read initial_max_data out of given clientToken");
    }

    underscore = ' ';
    clientTokenStream >> underscore;
    if (underscore != '_') {
        throw InvalidTokenException("Invalid format of given clientToken");
    }

    maxStreamData = 0;
    clientTokenStream >> maxStreamData;
    if (maxStreamData == 0) {
        throw InvalidTokenException("Cannot read initial_max_stream_data out of given clientToken");
    }

    initializeRemoteTransportParameters(maxData, maxStreamData);

    return token;
}

void Connection::addConnectionForInitialConnectionId(uint64_t initialConnectionId)
{
    this->initialConnectionIdSet = true;
    this->initialConnectionId = initialConnectionId;
    quicSimpleMod->addConnection(initialConnectionId, this);
}

void Connection::removeConnectionForInitialConnectionId()
{
    if (this->initialConnectionIdSet) {
        this->initialConnectionIdSet = false;
        quicSimpleMod->removeConnectionId(this->initialConnectionId);
    }
}

void Connection::initializeRemoteTransportParameters(Ptr<const TransportParametersExtension> transportParametersExt)
{
    remoteTransportParameters->readExtension(transportParametersExt);

    // set max data offset for the connection flow control and the flow control of each stream
    connectionFlowController->setMaxDataOffset(remoteTransportParameters->initialMaxData);
    for (auto it = streamMap.begin(); it != streamMap.end(); it++) {
        Stream *stream = it->second;
        stream->getFlowController()->setMaxDataOffset(remoteTransportParameters->initialMaxStreamData);
    }

    remoteTransportParametersInitialized = true;
}

void Connection::initializeRemoteTransportParameters(uint64_t maxData, uint64_t maxStreamData)
{
    remoteTransportParameters->initialMaxData = maxData;
    remoteTransportParameters->initialMaxStreamData = maxStreamData;

    // set max data offset for the connection flow control and the flow control of each stream
    connectionFlowController->setMaxDataOffset(remoteTransportParameters->initialMaxData);
    for (auto it = streamMap.begin(); it != streamMap.end(); it++) {
        Stream *stream = it->second;
        stream->getFlowController()->setMaxDataOffset(remoteTransportParameters->initialMaxStreamData);
    }

    remoteTransportParametersInitialized = true;
}

} /* namespace quic */
} /* namespace inet */
