/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 * Copyright (C) 2011 Zoltan Bojthe
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>

#include "inet/linklayer/ethernet/EtherMac.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/queue/IPassiveQueue.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ethernet/EtherEncap.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

// TODO: there is some code that is pretty much the same as the one found in EtherMacFullDuplex.cc (e.g. EtherMac::beginSendFrames)
// TODO: refactor using a statemachine that is present in a single function
// TODO: this helps understanding what interactions are there and how they affect the state

static std::ostream& operator<<(std::ostream& out, cMessage *msg)
{
    out << "(" << msg->getClassName() << ")" << msg->getFullName();
    return out;
}

Define_Module(EtherMac);

simsignal_t EtherMac::collisionSignal = registerSignal("collision");
simsignal_t EtherMac::backoffSlotsGeneratedSignal = registerSignal("backoffSlotsGenerated");

EtherMac::~EtherMac()
{
    delete frameBeingReceived;
    cancelAndDelete(endRxMsg);
    cancelAndDelete(endBackoffMsg);
    cancelAndDelete(endJammingMsg);
}

void EtherMac::initialize(int stage)
{
    EtherMacBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        endRxMsg = new cMessage("EndReception", ENDRECEPTION);
        endBackoffMsg = new cMessage("EndBackoff", ENDBACKOFF);
        endJammingMsg = new cMessage("EndJamming", ENDJAMMING);

        // initialize state info
        backoffs = 0;
        numConcurrentTransmissions = 0;
        currentSendPkTreeID = 0;

        WATCH(backoffs);
        WATCH(numConcurrentTransmissions);
    }
}

void EtherMac::initializeStatistics()
{
    EtherMacBase::initializeStatistics();

    framesSentInBurst = 0;
    bytesSentInBurst = B(0);

    WATCH(framesSentInBurst);
    WATCH(bytesSentInBurst);

    // initialize statistics
    totalCollisionTime = 0.0;
    totalSuccessfulRxTxTime = 0.0;
    numCollisions = numBackoffs = 0;

    WATCH(numCollisions);
    WATCH(numBackoffs);
}

void EtherMac::initializeFlags()
{
    EtherMacBase::initializeFlags();

    duplexMode = par("duplexMode");
    frameBursting = !duplexMode && par("frameBursting");
    physInGate->setDeliverOnReceptionStart(true);
}

void EtherMac::processConnectDisconnect()
{
    if (!connected) {
        delete frameBeingReceived;
        frameBeingReceived = nullptr;
        cancelEvent(endRxMsg);
        cancelEvent(endBackoffMsg);
        cancelEvent(endJammingMsg);
        bytesSentInBurst = B(0);
        framesSentInBurst = 0;
    }

    EtherMacBase::processConnectDisconnect();

    if (connected) {
        if (!duplexMode) {
            // start RX_RECONNECT_STATE
            changeReceptionState(RX_RECONNECT_STATE);
            simtime_t reconnectEndTime = simTime() + b(MAX_ETHERNET_FRAME_BYTES + JAM_SIGNAL_BYTES).get() / curEtherDescr->txrate;
            endRxTimeList.clear();
            addReceptionInReconnectState(-1, reconnectEndTime);
        }
    }
}

void EtherMac::readChannelParameters(bool errorWhenAsymmetric)
{
    EtherMacBase::readChannelParameters(errorWhenAsymmetric);

    if (connected && !duplexMode) {
        if (curEtherDescr->halfDuplexFrameMinBytes < B(0))
            throw cRuntimeError("%g bps Ethernet only supports full-duplex links", curEtherDescr->txrate);
    }
}

void EtherMac::handleSelfMessage(cMessage *msg)
{
    // Process different self-messages (timer signals)
    EV_TRACE << "Self-message " << msg << " received\n";

    switch (msg->getKind()) {
        case ENDIFG:
            handleEndIFGPeriod();
            break;

        case ENDTRANSMISSION:
            handleEndTxPeriod();
            break;

        case ENDRECEPTION:
            handleEndRxPeriod();
            break;

        case ENDBACKOFF:
            handleEndBackoffPeriod();
            break;

        case ENDJAMMING:
            handleEndJammingPeriod();
            break;

        case ENDPAUSE:
            handleEndPausePeriod();
            break;

        default:
            throw cRuntimeError("Self-message with unexpected message kind %d", msg->getKind());
    }
}

void EtherMac::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        handleMessageWhenDown(msg);
        return;
    }

    if (channelsDiffer)
        readChannelParameters(true);

    printState();

    // some consistency check
    if (!duplexMode && transmitState == TRANSMITTING_STATE && receiveState != RX_IDLE_STATE)
        throw cRuntimeError("Inconsistent state -- transmitting and receiving at the same time");

    if (msg->isSelfMessage())
        handleSelfMessage(msg);
    else if (msg->getArrivalGate() == upperLayerInGate)
        processFrameFromUpperLayer(check_and_cast<Packet *>(msg));
    else if (msg->getArrivalGate() == physInGate) {
        if (auto jamSignal = dynamic_cast<EthernetJamSignal *>(msg))
            processJamSignalFromNetwork(jamSignal);
        else
            processMsgFromNetwork(check_and_cast<EthernetSignal *>(msg));
    }
    else
        throw cRuntimeError("Message received from unknown gate");

    printState();
}

void EtherMac::processFrameFromUpperLayer(Packet *packet)
{
    ASSERT(packet->getDataLength() >= MIN_ETHERNET_FRAME_BYTES);

    EV_INFO << "Received " << packet << " from upper layer." << endl;

    numFramesFromHL++;
    emit(packetReceivedFromUpperSignal, packet);

    auto frame = packet->peekAtFront<EthernetMacHeader>();
    if (frame->getDest().equals(address)) {
        throw cRuntimeError("Logic error: frame %s from higher layer has local MAC address as dest (%s)",
                packet->getFullName(), frame->getDest().str().c_str());
    }

    if (packet->getDataLength() > MAX_ETHERNET_FRAME_BYTES) {
        throw cRuntimeError("Packet from higher layer (%d bytes) exceeds maximum Ethernet frame size (%d)",
                (int)(packet->getByteLength()), MAX_ETHERNET_FRAME_BYTES);
    }

    if (!connected || disabled) {
        EV_WARN << (!connected ? "Interface is not connected" : "MAC is disabled") << " -- dropping packet " << frame << endl;
        PacketDropDetails details;
        details.setReason(INTERFACE_DOWN);
        emit(packetDroppedSignal, packet, &details);
        numDroppedPkFromHLIfaceDown++;
        delete packet;

        requestNextFrameFromExtQueue();
        return;
    }

    // fill in src address if not set
    if (frame->getSrc().isUnspecified()) {
        //frame is immutable
        frame = nullptr;
        auto newFrame = packet->removeAtFront<EthernetMacHeader>();
        newFrame->setSrc(address);
        packet->insertAtFront(newFrame);
        frame = newFrame;
        auto oldFcs = packet->removeAtBack<EthernetFcs>();
        EtherEncap::addFcs(packet, oldFcs->getFcsMode());
    }

    if (txQueue.extQueue) {
        ASSERT(curTxFrame == nullptr);
        curTxFrame = packet;
        fillIFGIfInBurst();
    }
    else {
        if (txQueue.innerQueue->isFull())
            throw cRuntimeError("txQueue length exceeds %d -- this is probably due to "
                                "a bogus app model generating excessive traffic "
                                "(or if this is normal, increase txQueueLimit!)",
                    txQueue.innerQueue->getQueueLimit());

        // store frame and possibly begin transmitting
        EV_DETAIL << "Frame " << packet << " arrived from higher layer, enqueueing\n";
        txQueue.innerQueue->insertFrame(packet);

        if (!curTxFrame && !txQueue.innerQueue->isEmpty())
            curTxFrame = static_cast<Packet *>(txQueue.innerQueue->pop());
    }

    if ((duplexMode || receiveState == RX_IDLE_STATE) && transmitState == TX_IDLE_STATE) {
        EV_DETAIL << "No incoming carrier signals detected, frame clear to send\n";
        startFrameTransmission();
    }
}

void EtherMac::addReceptionInReconnectState(long packetTreeId, simtime_t endRxTime)
{
    // note: packetTreeId==-1 is legal, and represents a special entry that marks the end of the reconnect state

    // housekeeping: remove expired entries from endRxTimeList
    simtime_t now = simTime();
    while (!endRxTimeList.empty() && endRxTimeList.front().endTime <= now)
        endRxTimeList.pop_front();

    // remove old entry with same packet tree ID (typically: a frame reception
    // doesn't go through but is canceled by a jam signal)
    auto i = endRxTimeList.begin();
    for ( ; i != endRxTimeList.end(); i++) {
        if (i->packetTreeId == packetTreeId) {
            endRxTimeList.erase(i);
            break;
        }
    }

    // find insertion position and insert new entry (list is ordered by endRxTime)
    for (i = endRxTimeList.begin(); i != endRxTimeList.end() && i->endTime <= endRxTime; i++)
        ;
    PkIdRxTime item(packetTreeId, endRxTime);
    endRxTimeList.insert(i, item);

    // adjust endRxMsg if needed (we'll exit reconnect mode when endRxMsg expires)
    simtime_t maxRxTime = endRxTimeList.back().endTime;
    if (endRxMsg->getArrivalTime() != maxRxTime) {
        cancelEvent(endRxMsg);
        scheduleAt(maxRxTime, endRxMsg);
    }
}

void EtherMac::addReception(simtime_t endRxTime)
{
    numConcurrentTransmissions++;

    if (endRxMsg->getArrivalTime() < endRxTime) {
        cancelEvent(endRxMsg);
        scheduleAt(endRxTime, endRxMsg);
    }
}

void EtherMac::processReceivedJam(EthernetJamSignal *jam)
{
    simtime_t endRxTime = simTime() + jam->getDuration();
    delete jam;

    numConcurrentTransmissions--;
    if (numConcurrentTransmissions < 0)
        throw cRuntimeError("Received JAM without message");

    if (numConcurrentTransmissions == 0 || endRxMsg->getArrivalTime() < endRxTime) {
        cancelEvent(endRxMsg);
        scheduleAt(endRxTime, endRxMsg);
    }

    processDetectedCollision();
}

void EtherMac::processJamSignalFromNetwork(EthernetSignal *msg)
{
    EV_DETAIL << "Received " << msg << " from network.\n";

    if (!connected || disabled) {
        EV_WARN << (!connected ? "Interface is not connected" : "MAC is disabled") << " -- dropping msg " << msg << endl;
        delete msg;
        return;
    }

    // detect cable length violation in half-duplex mode
    if (!duplexMode) {
        simtime_t propagationTime = simTime() - msg->getSendingTime();
        if (propagationTime >= curEtherDescr->maxPropagationDelay) {
            throw cRuntimeError("Very long frame propagation time detected, maybe cable exceeds "
                                "maximum allowed length? (%lgs corresponds to an approx. %lgm cable)",
                    SIMTIME_STR(propagationTime),
                    SIMTIME_STR(propagationTime * SPEED_OF_LIGHT_IN_CABLE));
        }
    }

    simtime_t endRxTime = simTime() + msg->getDuration();
    EthernetJamSignal *jamMsg = dynamic_cast<EthernetJamSignal *>(msg);

    if (duplexMode && jamMsg) {
        throw cRuntimeError("Stray jam signal arrived in full-duplex mode");
    }
    else if (!duplexMode && receiveState == RX_RECONNECT_STATE) {
        long treeId = jamMsg->getAbortedPkTreeID();
        addReceptionInReconnectState(treeId, endRxTime);
        delete msg;
    }
    else if (!duplexMode && (transmitState == TRANSMITTING_STATE || transmitState == SEND_IFG_STATE)) {
        // since we're half-duplex, receiveState must be RX_IDLE_STATE (asserted at top of handleMessage)
        if (jamMsg)
            throw cRuntimeError("Stray jam signal arrived while transmitting (usual cause is cable length exceeding allowed maximum)");
    }
    else if (receiveState == RX_IDLE_STATE) {
        if (jamMsg)
            throw cRuntimeError("Stray jam signal arrived (usual cause is cable length exceeding allowed maximum)");
    }
    else {    // (receiveState==RECEIVING_STATE || receiveState==RX_COLLISION_STATE)
              // handle overlapping receptions
        processReceivedJam(jamMsg);
    }
}

void EtherMac::processMsgFromNetwork(EthernetSignal *signal)
{
    EV_DETAIL << "Received " << signal << " from network.\n";

    if (!connected || disabled) {
        EV_WARN << (!connected ? "Interface is not connected" : "MAC is disabled") << " -- dropping msg " << signal << endl;
        if (typeid(*signal) == typeid(EthernetSignal)) {    // do not count JAM and IFG packets
            auto packet = check_and_cast<Packet *>(signal->decapsulate());
            delete signal;
            decapsulate(packet);
            PacketDropDetails details;
            details.setReason(INTERFACE_DOWN);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
            numDroppedIfaceDown++;
        }
        else
            delete signal;

        return;
    }

    // detect cable length violation in half-duplex mode
    if (!duplexMode) {
        simtime_t propagationTime = simTime() - signal->getSendingTime();
        if (propagationTime >= curEtherDescr->maxPropagationDelay) {
            throw cRuntimeError("Very long frame propagation time detected, maybe cable exceeds "
                                "maximum allowed length? (%lgs corresponds to an approx. %lgm cable)",
                    SIMTIME_STR(propagationTime),
                    SIMTIME_STR(propagationTime * SPEED_OF_LIGHT_IN_CABLE));
        }
    }

    simtime_t endRxTime = simTime() + signal->getDuration();

    if (!duplexMode && receiveState == RX_RECONNECT_STATE) {
        long treeId = signal->getTreeId();
        addReceptionInReconnectState(treeId, endRxTime);
        delete signal;
    }
    else if (!duplexMode && (transmitState == TRANSMITTING_STATE || transmitState == SEND_IFG_STATE)) {
        // since we're half-duplex, receiveState must be RX_IDLE_STATE (asserted at top of handleMessage)
        // set receive state and schedule end of reception
        changeReceptionState(RX_COLLISION_STATE);

        addReception(endRxTime);
        delete signal;

        EV_DETAIL << "Transmission interrupted by incoming frame, handling collision\n";
        cancelEvent((transmitState == TRANSMITTING_STATE) ? endTxMsg : endIFGMsg);

        EV_DETAIL << "Transmitting jam signal\n";
        sendJamSignal();    // backoff will be executed when jamming finished

        numCollisions++;
        emit(collisionSignal, 1L);
    }
    else if (receiveState == RX_IDLE_STATE) {
        channelBusySince = simTime();
        EV_INFO << "Reception of " << signal << " started.\n";
        scheduleEndRxPeriod(signal);
    }
    else if (receiveState == RECEIVING_STATE && endRxMsg->getArrivalTime() - simTime() < curEtherDescr->halfBitTime)
    {
        // With the above condition we filter out "false" collisions that may occur with
        // back-to-back frames. That is: when "beginning of frame" message (this one) occurs
        // BEFORE "end of previous frame" event (endRxMsg) -- same simulation time,
        // only wrong order.

        EV_DETAIL << "Back-to-back frames: completing reception of current frame, starting reception of next one\n";

        // complete reception of previous frame
        cancelEvent(endRxMsg);
        frameReceptionComplete();

        // calculate usability
        totalSuccessfulRxTxTime += simTime() - channelBusySince;
        channelBusySince = simTime();

        // start receiving next frame
        scheduleEndRxPeriod(signal);
    }
    else {    // (receiveState==RECEIVING_STATE || receiveState==RX_COLLISION_STATE)
              // handle overlapping receptions
        // EtherFrame or EtherPauseFrame
        EV_DETAIL << "Overlapping receptions -- setting collision state\n";
        addReception(endRxTime);
        // delete collided frames: arrived frame as well as the one we're currently receiving
        delete signal;
        processDetectedCollision();
    }
}

void EtherMac::processDetectedCollision()
{
    if (receiveState != RX_COLLISION_STATE) {
        delete frameBeingReceived;
        frameBeingReceived = nullptr;

        numCollisions++;
        emit(collisionSignal, 1L);
        // go to collision state
        changeReceptionState(RX_COLLISION_STATE);
    }
}

void EtherMac::handleEndIFGPeriod()
{
    if (transmitState != WAIT_IFG_STATE && transmitState != SEND_IFG_STATE)
        throw cRuntimeError("Not in WAIT_IFG_STATE at the end of IFG period");

    currentSendPkTreeID = 0;

    EV_DETAIL << "IFG elapsed\n";

    if (frameBursting && (transmitState != SEND_IFG_STATE)) {
        bytesSentInBurst = B(0);
        framesSentInBurst = 0;
    }

    // End of IFG period, okay to transmit, if Rx idle OR duplexMode ( checked in startFrameTransmission(); )

    // send frame to network
    beginSendFrames();
}

void EtherMac::startFrameTransmission()
{
    ASSERT(curTxFrame);

    EV_INFO << "Transmission of " << curTxFrame << " started.\n";

    Packet *frame = curTxFrame->dup();

    const auto& hdr = frame->peekAtFront<EthernetMacHeader>();
    ASSERT(hdr);
    ASSERT(!hdr->getSrc().isUnspecified());

    bool inBurst = frameBursting && framesSentInBurst;
    B minFrameLength = duplexMode ? curEtherDescr->frameMinBytes : (inBurst ? curEtherDescr->frameInBurstMinBytes : curEtherDescr->halfDuplexFrameMinBytes);

    if (frame->getDataLength() < minFrameLength) {
        auto oldFcs = frame->removeAtBack<EthernetFcs>();
        EtherEncap::addPaddingAndFcs(frame, oldFcs->getFcsMode(), minFrameLength);
    }

    // add preamble and SFD (Starting Frame Delimiter), then send out
    encapsulate(frame);

    B sentFrameByteLength = B(frame->getByteLength());
    auto oldPacketProtocolTag = frame->removeTag<PacketProtocolTag>();
    frame->clearTags();
    auto newPacketProtocolTag = frame->addTag<PacketProtocolTag>();
    *newPacketProtocolTag = *oldPacketProtocolTag;
    delete oldPacketProtocolTag;
    auto signal = new EthernetSignal(frame->getName());
    currentSendPkTreeID = signal->getTreeId();
    if (sendRawBytes) {
        auto rawFrame = new Packet(frame->getName(), frame->peekAllAsBytes());
        rawFrame->copyTags(*frame);
        signal->encapsulate(rawFrame);
        delete frame;
    }
    else
        signal->encapsulate(frame);
    send(signal, physOutGate);

    // check for collisions (there might be an ongoing reception which we don't know about, see below)
    if (!duplexMode && receiveState != RX_IDLE_STATE) {
        // During the IFG period the hardware cannot listen to the channel,
        // so it might happen that receptions have begun during the IFG,
        // and even collisions might be in progress.
        //
        // But we don't know of any ongoing transmission so we blindly
        // start transmitting, immediately collide and send a jam signal.
        //
        EV_DETAIL << "startFrameTransmission(): sending JAM signal.\n";
        printState();

        sendJamSignal();
        // numConcurrentRxTransmissions stays the same: +1 transmission, -1 jam

        if (receiveState == RECEIVING_STATE) {
            delete frameBeingReceived;
            frameBeingReceived = nullptr;

            numCollisions++;
            emit(collisionSignal, 1L);
        }
        // go to collision state
        changeReceptionState(RX_COLLISION_STATE);
    }
    else {
        // no collision
        scheduleEndTxPeriod(sentFrameByteLength);

        // only count transmissions in totalSuccessfulRxTxTime if channel is half-duplex
        if (!duplexMode)
            channelBusySince = simTime();
    }
}

void EtherMac::handleEndTxPeriod()
{
    // we only get here if transmission has finished successfully, without collision
    if (transmitState != TRANSMITTING_STATE || (!duplexMode && receiveState != RX_IDLE_STATE))
        throw cRuntimeError("End of transmission, and incorrect state detected");

    currentSendPkTreeID = 0;

    if (curTxFrame == nullptr)
        throw cRuntimeError("Frame under transmission cannot be found");

    numFramesSent++;
    numBytesSent += curTxFrame->getByteLength();
    emit(packetSentToLowerSignal, curTxFrame);    //consider: emit with start time of frame

    const auto& header = curTxFrame->peekAtFront<EthernetMacHeader>();
    if (header->getTypeOrLength() == ETHERTYPE_FLOW_CONTROL) {
        const auto& controlFrame = curTxFrame->peekDataAt<EthernetControlFrame>(header->getChunkLength(), b(-1));
        if (controlFrame->getOpCode() == ETHERNET_CONTROL_PAUSE) {
            const auto& pauseFrame = dynamicPtrCast<const EthernetPauseFrame>(controlFrame);
            numPauseFramesSent++;
            emit(txPausePkUnitsSignal, pauseFrame->getPauseTime());
        }
    }

    EV_INFO << "Transmission of " << curTxFrame << " successfully completed.\n";
    delete curTxFrame;
    curTxFrame = nullptr;
    lastTxFinishTime = simTime();
    getNextFrameFromQueue();  // note: cannot be moved into handleEndIFGPeriod(), because in burst mode we need to know whether to send filled IFG or not

    // only count transmissions in totalSuccessfulRxTxTime if channel is half-duplex
    if (!duplexMode) {
        simtime_t dt = simTime() - channelBusySince;
        totalSuccessfulRxTxTime += dt;
    }

    backoffs = 0;

    // check for and obey received PAUSE frames after each transmission
    if (pauseUnitsRequested > 0) {
        // if we received a PAUSE frame recently, go into PAUSE state
        EV_DETAIL << "Going to PAUSE mode for " << pauseUnitsRequested << " time units\n";
        scheduleEndPausePeriod(pauseUnitsRequested);
        pauseUnitsRequested = 0;
    }
    else {
        EV_DETAIL << "Start IFG period\n";
        scheduleEndIFGPeriod();
        if (!txQueue.extQueue)
            fillIFGIfInBurst();
    }
}

void EtherMac::scheduleEndRxPeriod(EthernetSignal *frame)
{
    ASSERT(frameBeingReceived == nullptr);
    ASSERT(!endRxMsg->isScheduled());

    frameBeingReceived = frame;
    changeReceptionState(RECEIVING_STATE);
    addReception(simTime() + frame->getDuration());
}

void EtherMac::handleEndRxPeriod()
{
    simtime_t dt = simTime() - channelBusySince;

    switch (receiveState) {
        case RECEIVING_STATE:
            frameReceptionComplete();
            totalSuccessfulRxTxTime += dt;
            break;

        case RX_COLLISION_STATE:
            EV_DETAIL << "Incoming signals finished after collision\n";
            totalCollisionTime += dt;
            break;

        case RX_RECONNECT_STATE:
            EV_DETAIL << "Incoming signals finished or reconnect time elapsed after reconnect\n";
            endRxTimeList.clear();
            break;

        default:
            throw cRuntimeError("model error: invalid receiveState %d", receiveState);
    }

    changeReceptionState(RX_IDLE_STATE);
    numConcurrentTransmissions = 0;

    if (transmitState == TX_IDLE_STATE)
        scheduleEndIFGPeriod();
}

void EtherMac::handleEndBackoffPeriod()
{
    if (transmitState != BACKOFF_STATE)
        throw cRuntimeError("At end of BACKOFF and not in BACKOFF_STATE");

    if (curTxFrame == nullptr)
        throw cRuntimeError("At end of BACKOFF and no frame to transmit");

    if (receiveState == RX_IDLE_STATE) {
        EV_DETAIL << "Backoff period ended, wait IFG\n";
        scheduleEndIFGPeriod();
    }
    else {
        EV_DETAIL << "Backoff period ended but channel is not free, idling\n";
        changeTransmissionState(TX_IDLE_STATE);
    }
}

void EtherMac::sendJamSignal()
{
    if (currentSendPkTreeID == 0)
        throw cRuntimeError("Model error: sending JAM while not transmitting");

    EthernetJamSignal *jam = new EthernetJamSignal("JAM_SIGNAL");
    jam->setByteLength(B(JAM_SIGNAL_BYTES).get());
    jam->setAbortedPkTreeID(currentSendPkTreeID);

    transmissionChannel->forceTransmissionFinishTime(SIMTIME_ZERO);
    //emit(packetSentToLowerSignal, jam);
    send(jam, physOutGate);

    scheduleAt(transmissionChannel->getTransmissionFinishTime(), endJammingMsg);
    changeTransmissionState(JAMMING_STATE);
}

void EtherMac::handleEndJammingPeriod()
{
    if (transmitState != JAMMING_STATE)
        throw cRuntimeError("At end of JAMMING but not in JAMMING_STATE");

    EV_DETAIL << "Jamming finished, executing backoff\n";
    handleRetransmission();
}

void EtherMac::handleRetransmission()
{
    if (++backoffs > MAX_ATTEMPTS) {
        EV_DETAIL << "Number of retransmit attempts of frame exceeds maximum, cancelling transmission of frame\n";
        PacketDropDetails details;
        details.setReason(RETRY_LIMIT_REACHED);
        details.setLimit(MAX_ATTEMPTS);
        emit(packetDroppedSignal, curTxFrame, &details);
        delete curTxFrame;
        curTxFrame = nullptr;
        changeTransmissionState(TX_IDLE_STATE);
        backoffs = 0;
        getNextFrameFromQueue();
        beginSendFrames();
        return;
    }

    EV_DETAIL << "Executing backoff procedure\n";
    int backoffRange = (backoffs >= BACKOFF_RANGE_LIMIT) ? 1024 : (1 << backoffs);
    int slotNumber = intuniform(0, backoffRange - 1);

    scheduleAt(simTime() + slotNumber * curEtherDescr->slotTime, endBackoffMsg);
    changeTransmissionState(BACKOFF_STATE);
    emit(backoffSlotsGeneratedSignal, slotNumber);

    numBackoffs++;
}

void EtherMac::printState()
{
#define CASE(x)    case x: \
        EV_DETAIL << #x; break

    EV_DETAIL << "transmitState: ";
    switch (transmitState) {
        CASE(TX_IDLE_STATE);
        CASE(WAIT_IFG_STATE);
        CASE(SEND_IFG_STATE);
        CASE(TRANSMITTING_STATE);
        CASE(JAMMING_STATE);
        CASE(BACKOFF_STATE);
        CASE(PAUSE_STATE);
    }

    EV_DETAIL << ",  receiveState: ";
    switch (receiveState) {
        CASE(RX_IDLE_STATE);
        CASE(RECEIVING_STATE);
        CASE(RX_COLLISION_STATE);
        CASE(RX_RECONNECT_STATE);
    }

    EV_DETAIL << ",  backoffs: " << backoffs;
    EV_DETAIL << ",  numConcurrentRxTransmissions: " << numConcurrentTransmissions;

    if (txQueue.innerQueue)
        EV_DETAIL << ",  queueLength: " << txQueue.innerQueue->getLength();

    EV_DETAIL << endl;

#undef CASE
}

void EtherMac::finish()
{
    EtherMacBase::finish();

    simtime_t t = simTime();
    simtime_t totalChannelIdleTime = t - totalSuccessfulRxTxTime - totalCollisionTime;
    recordScalar("rx channel idle (%)", 100 * (totalChannelIdleTime / t));
    recordScalar("rx channel utilization (%)", 100 * (totalSuccessfulRxTxTime / t));
    recordScalar("rx channel collision (%)", 100 * (totalCollisionTime / t));
    recordScalar("collisions", numCollisions);
    recordScalar("backoffs", numBackoffs);
}

void EtherMac::handleEndPausePeriod()
{
    if (transmitState != PAUSE_STATE)
        throw cRuntimeError("At end of PAUSE and not in PAUSE_STATE");

    EV_DETAIL << "Pause finished, resuming transmissions\n";
    beginSendFrames();
}

void EtherMac::frameReceptionComplete()
{
    EthernetSignal *signal = frameBeingReceived;
    frameBeingReceived = nullptr;

    if (dynamic_cast<EthernetFilledIfgSignal *>(signal) != nullptr) {
        delete signal;
        return;
    }
    bool hasBitError = signal->hasBitError();
    auto packet = check_and_cast<Packet *>(signal->decapsulate());
    delete signal;
    decapsulate(packet);
    emit(packetReceivedFromLowerSignal, packet);

    if (hasBitError || !verifyCrcAndLength(packet)) {
        numDroppedBitError++;
        PacketDropDetails details;
        details.setReason(INCORRECTLY_RECEIVED);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }

    const auto& frame = packet->peekAtFront<EthernetMacHeader>();
    if (dropFrameNotForUs(packet, frame))
        return;

    if (frame->getTypeOrLength() == ETHERTYPE_FLOW_CONTROL) {
        processReceivedControlFrame(packet);
    }
    else {
        EV_INFO << "Reception of " << frame << " successfully completed.\n";
        processReceivedDataFrame(packet);
    }
}

void EtherMac::processReceivedDataFrame(Packet *packet)
{
    // statistics
    unsigned long curBytes = packet->getByteLength();
    numFramesReceivedOK++;
    numBytesReceivedOK += curBytes;
    emit(rxPkOkSignal, packet);

    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ethernetMac);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
    if (interfaceEntry)
        packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());

    numFramesPassedToHL++;
    emit(packetSentToUpperSignal, packet);
    // pass up to upper layer
    EV_INFO << "Sending " << packet << " to upper layer.\n";
    send(packet, "upperLayerOut");
}

void EtherMac::processReceivedControlFrame(Packet *packet)
{
    packet->popAtFront<EthernetMacHeader>();
    const auto& controlFrame = packet->peekAtFront<EthernetControlFrame>();

    if (controlFrame->getOpCode() == ETHERNET_CONTROL_PAUSE) {
        const auto& pauseFrame = packet->peekAtFront<EthernetPauseFrame>();
        int pauseUnits = pauseFrame->getPauseTime();

        numPauseFramesRcvd++;
        emit(rxPausePkUnitsSignal, pauseUnits);

        if (transmitState == TX_IDLE_STATE) {
            EV_DETAIL << "PAUSE frame received, pausing for " << pauseUnitsRequested << " time units\n";
            if (pauseUnits > 0)
                scheduleEndPausePeriod(pauseUnits);
        }
        else if (transmitState == PAUSE_STATE) {
            EV_DETAIL << "PAUSE frame received, pausing for " << pauseUnitsRequested
                      << " more time units from now\n";
            cancelEvent(endPauseMsg);

            if (pauseUnits > 0)
                scheduleEndPausePeriod(pauseUnits);
        }
        else {
            // transmitter busy -- wait until it finishes with current frame (endTx)
            // and then it'll go to PAUSE state
            EV_DETAIL << "PAUSE frame received, storing pause request\n";
            pauseUnitsRequested = pauseUnits;
        }
    }
    delete packet;
}

void EtherMac::scheduleEndIFGPeriod()
{
    changeTransmissionState(WAIT_IFG_STATE);
    simtime_t endIFGTime = simTime() + (b(INTERFRAME_GAP_BITS).get() / curEtherDescr->txrate);
    scheduleAt(endIFGTime, endIFGMsg);
}

void EtherMac::fillIFGIfInBurst()
{
    if (!frameBursting)
        return;

    EV_TRACE << "fillIFGIfInBurst(): t=" << simTime() << ", framesSentInBurst=" << framesSentInBurst << ", bytesSentInBurst=" << bytesSentInBurst << endl;

    if (curTxFrame
        && endIFGMsg->isScheduled()
        && (transmitState == WAIT_IFG_STATE)
        && (simTime() == lastTxFinishTime)
        && (simTime() == endIFGMsg->getSendingTime())
        && (framesSentInBurst > 0)
        && (framesSentInBurst < curEtherDescr->maxFramesInBurst)
        && (bytesSentInBurst + INTERFRAME_GAP_BITS + PREAMBLE_BYTES + SFD_BYTES + curTxFrame->getDataLength()
            <= curEtherDescr->maxBytesInBurst)
        )
    {
        EthernetFilledIfgSignal *gap = new EthernetFilledIfgSignal("FilledIFG");
        bytesSentInBurst += B(gap->getByteLength());
        currentSendPkTreeID = gap->getTreeId();
        send(gap, physOutGate);
        changeTransmissionState(SEND_IFG_STATE);
        cancelEvent(endIFGMsg);
        scheduleAt(transmissionChannel->getTransmissionFinishTime(), endIFGMsg);
    }
    else {
        bytesSentInBurst = B(0);
        framesSentInBurst = 0;
    }
}

void EtherMac::scheduleEndTxPeriod(B sentFrameByteLength)
{
    // update burst variables
    if (frameBursting) {
        bytesSentInBurst += sentFrameByteLength;
        framesSentInBurst++;
    }

    scheduleAt(transmissionChannel->getTransmissionFinishTime(), endTxMsg);
    changeTransmissionState(TRANSMITTING_STATE);
}

void EtherMac::scheduleEndPausePeriod(int pauseUnits)
{
    // length is interpreted as 512-bit-time units
    simtime_t pausePeriod = pauseUnits * PAUSE_UNIT_BITS / curEtherDescr->txrate;
    scheduleAt(simTime() + pausePeriod, endPauseMsg);
    changeTransmissionState(PAUSE_STATE);
}

void EtherMac::beginSendFrames()
{
    if (curTxFrame) {
        // Other frames are queued, therefore wait IFG period and transmit next frame
        EV_DETAIL << "Will transmit next frame in output queue after IFG period\n";
        startFrameTransmission();
    }
    else {
        // No more frames, set transmitter to idle
        changeTransmissionState(TX_IDLE_STATE);
        EV_DETAIL << "No more frames to send, transmitter set to idle\n";
    }
}

} // namespace inet

