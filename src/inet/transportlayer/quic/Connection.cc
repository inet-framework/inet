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

namespace inet {
namespace quic {

Connection::Connection(Quic *quicSimpleMod, UdpSocket *udpSocket, AppSocket *appSocket, L3Address remoteAddr, int remotePort) {
    this->quicSimpleMod = quicSimpleMod;
    this->udpSocket = udpSocket;
    this->appSocket = appSocket;

    bool useDplpmtud = quicSimpleMod->par("useDplpmtud");

    connectionIds.push_back(0);
    mainConnectionId = 0;

    stats = new Statistics(quicSimpleMod, "_cid=" + std::to_string(mainConnectionId));

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

    transportParameter = new TransportParameter(quicSimpleMod);

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
    receivedPacketsAccountant = new ReceivedPacketsAccountant(createTimer(TimerType::ACK_DELAY_TIMER, "AckDelayTimer"), transportParameter);
    receivedPacketsAccountant->readParameters(quicSimpleMod);
    packetBuilder = new PacketBuilder(&controlQueue, scheduler, receivedPacketsAccountant);
    packetBuilder->setConnectionId(mainConnectionId);
    packetBuilder->readParameters(quicSimpleMod);

    congestionController = CongestionControlFactory::getInstance()->createCongestionController(quicSimpleMod->par("congestionControl"));
    congestionController->readParameters(quicSimpleMod);
    congestionController->setStatistics(stats);
    congestionController->setPath(path);

    reliabilityManager = new ReliabilityManager(this, transportParameter, receivedPacketsAccountant, congestionController, stats);

    connectionFlowController = new ConnectionFlowController(transportParameter->initial_max_data, stats);
    connectionFlowControlResponder = new ConnectionFlowControlResponder(this, transportParameter->initial_max_data, maxDataFrameThreshold, roundConsumedDataValue, stats);
}

Connection::~Connection() {
    delete reliabilityManager;
    delete receivedPacketsAccountant;
    delete congestionController;
    delete connectionFlowController;
    delete connectionFlowControlResponder;
    delete stats;
    delete connectionState;
    delete transportParameter;
    delete scheduler;
    delete packetBuilder;
    delete path;

    for (QuicFrame *frame : controlQueue) {
        delete frame;
    }
    // delete stream objects
    for (auto it = streamMap.begin(); it != streamMap.end(); it++) {
        delete it->second;
    }
}

void Connection::processAppCommand(cMessage *msg)
{
    ConnectionState *newState = connectionState->processAppCommand(msg);
    if (newState != connectionState) {
        // state transition
        delete connectionState;
        connectionState = newState;
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

/**
 * Enqueues data in the corresponding stream queue and triggers packet sending.
 * Called upon a send command from app.
 * \param streamId Id of the stream.
 * \param data Data to enqueue.
 */
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
        }
        if (pkt->getByteLength() >= byteLengthBefore) {
            throw cRuntimeError("no packet processing happened");
        }
    }
}

void Connection::processTimeout(cMessage *msg)
{
    ConnectionState *newState = connectionState->processTimeout(msg);
    if (newState != connectionState) {
        // state transition
        delete connectionState;
        connectionState = newState;
    }
}

void Connection::processIcmpPtb(Packet *droppedPkt, int ptbMtu)
{
    ConnectionState *newState = connectionState->processIcmpPtb(droppedPkt, ptbMtu);
    if (newState != connectionState) {
        // state transition
        delete connectionState;
        connectionState = newState;
    }
}

void Connection::sendPackets()
{
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
            packet = packetBuilder->buildPacket(maxQuicPacketSize, path->getSafeQuicPacketSize());
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
        sendPacket(packet); // updates cwnd
    }
    if (receivedPacketsAccountant->wantsToSendAckImmediately()) {
        // ReceivedPacketsAccountant still needs to send an ACK frame
        // CongestionController might prohibit sending, but we can still send an Ack-only-packet
        // restrict the size of the packet by a safe upper limit, because for a non-ack-eliciting packet we have no PMTU validation.
        QuicPacket *packet = packetBuilder->buildAckOnlyPacket(path->getSafeQuicPacketSize());
        sendPacket(packet);
    }
}

/**
 * Creates a probe packet (aka tail loss probe) by using
 * (1) new data,
 * (2) retransmit sent but outstanding data, or
 * (3) a ping frame.
 * After that, it sends the probe packet.
 * This method is used by ReliabilityManager when its lossDetectionTimer fires.
 */
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
        std::vector<QuicPacket*> *ackElicitingSentPackets = reliabilityManager->getAckElicitingSentPackets();
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
    sendPacket(packet);
}

void Connection::sendPacket(QuicPacket *packet)
{
    stats->getMod()->emit(packetNumberSentStat, packet->getPacketNumber());
    Packet *pkt = packet->createOmnetPacket();
    stats->getMod()->emit(packetSentSignal, pkt);
    udpSocket->sendto(path->getRemoteAddr(), path->getRemotePort(), pkt);
    reliabilityManager->onPacketSent(packet);
}

std::vector<uint64_t> Connection::getConnectionIds()
{
    return this->connectionIds;
}

void Connection::connect()
{
    sendPacket(packetBuilder->buildInitialPacket());
    /*
    appSocket->sendEstablished();

    if (path->usesDplpmtud()) {
        path->getDplpmtud()->start();
    }
    */
}

void Connection::accept()
{
    if (path->usesDplpmtud()) {
        path->getDplpmtud()->start();
    }
}

void Connection::measureReceiveGoodput(uint64_t receivedDataLength)
{
    static simsignal_t receiveGoodputStat = stats->createStatisticEntry("receiveGoodput");
    static SimTime empty = SimTime(-1.0);
    static SimTime receiveGoodputStatStartTime = SimTime(quicSimpleMod->par("receiveGoodputStatStartTime"));
    static SimTime receiveGoodputStatEndTime = SimTime(quicSimpleMod->par("receiveGoodputStatEndTime"));
    static bool receiveGoodputDone = (receiveGoodputStatStartTime == empty);
    static SimTime firstReceivedDataTime = empty;
    static double totalReceivedDataLength = 0.0;

    if (!receiveGoodputDone) {
        SimTime now = simTime();
        if (receiveGoodputStatStartTime <= now) {
            if (firstReceivedDataTime == empty) {
                firstReceivedDataTime = now;
            } else {
                totalReceivedDataLength += receivedDataLength;
                if ((now - firstReceivedDataTime).dbl() > 0) {
                    stats->getMod()->emit(receiveGoodputStat, totalReceivedDataLength / (now - firstReceivedDataTime).dbl() * 8.0);
                }
                if (receiveGoodputStatEndTime <= now && receiveGoodputStatEndTime != empty) {
                    receiveGoodputDone = true;
                }
            }
        }
    }
}

void Connection::processReceivedData(uint64_t streamId, uint64_t offset, Ptr<const Chunk> data)
{
    Stream *stream = findOrCreateStream(streamId);
    uint64_t dataLength = B(data->getChunkLength()).get();

    static simsignal_t totalReceivedDataBytesStat = stats->createStatisticEntry("totalReceivedDataBytes");
    static unsigned long totalReceivedDataBytes = 0;
    totalReceivedDataBytes += dataLength;
    stats->getMod()->emit(totalReceivedDataBytesStat, totalReceivedDataBytes);
    measureReceiveGoodput(dataLength);

    //check rwnd before process data
    if(stream->isAllowedToReceivedData(dataLength)){

        stream->bufferReceivedData(data, offset);
        stream->updateHighestRecievedOffset(offset + dataLength);
        stream->measureStreamRcvDataBytes(dataLength);
        stream->measureStreamGoodput(SimTime(quicSimpleMod->par("receiveGoodputStatStartTime")), simTime());

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

void Connection::accountReceivedPacket(uint64_t packetNumber, bool ackEliciting, bool isIBitSet)
{
    stats->getMod()->emit(packetNumberReceivedStat, packetNumber);
    receivedPacketsAccountant->onPacketReceived(packetNumber, ackEliciting, isIBitSet);
    if (receivedPacketsAccountant->wantsToSendAckImmediately()) {
        sendPackets();
    }
}

/**
 * Creates a timer with the given type and name.
 * \param kind Kind of the timer message.
 * \param name Name of the timer message.
 * \return Pointer to the created Timer object.
 */
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

void Connection::handleAckFrame(const Ptr<const AckFrameHeader>& frameHeader)
{
    reliabilityManager->onAckReceived(frameHeader);

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

void Connection::reportPtb(int droppedPacketNumber, int ptbMtu)
{
    QuicPacket *packet = reliabilityManager->getSentPacket(droppedPacketNumber);
    if (packet == nullptr) {
        EV_WARN << "could not find sent packet that matches to the packet reflected inside the ICMP packet" << endl;
        return;
    }

    if (path->usesDplpmtud()) {
        path->getDplpmtud()->onPtbReceived(packet->getSize(), ptbMtu);
    }
}

} /* namespace quic */
} /* namespace inet */
