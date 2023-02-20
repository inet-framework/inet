//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
// Copyright (C) 2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/basic/EthernetCsmaMac.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/ethernet/common/EthernetControlFrame_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wired/ethernet/EthernetSignal_m.h"

namespace inet {

// TODO there is some code that is pretty much the same as the one found in EthernetMac.cc (e.g. EthernetCsmaMac::beginSendFrames)
// TODO refactor using a statemachine that is present in a single function
// TODO this helps understanding what interactions are there and how they affect the state

static std::ostream& operator<<(std::ostream& out, cMessage *msg)
{
    out << "(" << msg->getClassName() << ")" << msg->getFullName();
    return out;
}

Define_Module(EthernetCsmaMac);

simsignal_t EthernetCsmaMac::collisionSignal = registerSignal("collision");
simsignal_t EthernetCsmaMac::backoffSlotsGeneratedSignal = registerSignal("backoffSlotsGenerated");

EthernetCsmaMac::~EthernetCsmaMac()
{
    for (auto& rx : rxSignals)
        delete rx.signal;
    cancelAndDelete(endRxTimer);
    cancelAndDelete(endBackoffTimer);
    cancelAndDelete(endJammingTimer);
}

void EthernetCsmaMac::initialize(int stage)
{
    EthernetMacBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        endRxTimer = new cMessage("EndReception", ENDRECEPTION);
        endBackoffTimer = new cMessage("EndBackoff", ENDBACKOFF);
        endJammingTimer = new cMessage("EndJamming", ENDJAMMING);

        // initialize state info
        backoffs = 0;
        setTxUpdateSupport(true);

        WATCH(backoffs);
    }
}

void EthernetCsmaMac::initializeStatistics()
{
    EthernetMacBase::initializeStatistics();

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

void EthernetCsmaMac::initializeFlags()
{
    EthernetMacBase::initializeFlags();

    duplexMode = par("duplexMode");
    frameBursting = !duplexMode && par("frameBursting");
    physInGate->setDeliverImmediately(true);
}

void EthernetCsmaMac::processConnectDisconnect()
{
    if (!connected) {
        for (auto& rx : rxSignals)
            delete rx.signal;
        rxSignals.clear();
        cancelEvent(endRxTimer);
        cancelEvent(endBackoffTimer);
        cancelEvent(endJammingTimer);
        bytesSentInBurst = B(0);
        framesSentInBurst = 0;
    }

    EthernetMacBase::processConnectDisconnect();

    if (connected) {
        if (!duplexMode) {
            // start RX_RECONNECT_STATE
            changeReceptionState(RX_RECONNECT_STATE);
            simtime_t reconnectEndTime = simTime() + b(MAX_ETHERNET_FRAME_BYTES + JAM_SIGNAL_BYTES).get() / curEtherDescr->txrate;
            for (auto& rx : rxSignals)
                delete rx.signal;
            rxSignals.clear();
            EthernetSignalBase *signal = new EthernetSignalBase("RECONNECT");
            signal->setBitError(true);
            signal->setDuration(reconnectEndTime);
            updateRxSignals(signal, reconnectEndTime);
        }
    }
}

void EthernetCsmaMac::readChannelParameters(bool errorWhenAsymmetric)
{
    EthernetMacBase::readChannelParameters(errorWhenAsymmetric);

    if (connected && !duplexMode) {
        if (curEtherDescr->halfDuplexFrameMinBytes < B(0))
            throw cRuntimeError("%g bps Ethernet only supports full-duplex links", curEtherDescr->txrate);
    }
}

void EthernetCsmaMac::handleSelfMessage(cMessage *msg)
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

void EthernetCsmaMac::handleMessageWhenUp(cMessage *msg)
{
    if (channelsDiffer)
        readChannelParameters(true);

    printState();

    // some consistency check
    if (!duplexMode && transmitState == TRANSMITTING_STATE && receiveState != RX_IDLE_STATE)
        throw cRuntimeError("Inconsistent state -- transmitting and receiving at the same time");

    if (msg->isSelfMessage())
        handleSelfMessage(msg);
    else if (msg->getArrivalGateId() == upperLayerInGateId)
        handleUpperPacket(check_and_cast<Packet *>(msg));
    else if (msg->getArrivalGate() == physInGate)
        handleSignalFromNetwork(check_and_cast<EthernetSignalBase *>(msg));
    else
        throw cRuntimeError("Message received from unknown gate");

    processAtHandleMessageFinished();
    printState();
}

void EthernetCsmaMac::handleSignalFromNetwork(EthernetSignalBase *signal)
{
    EV_DETAIL << "Received " << signal << " from network.\n";

    if (!connected) {
        EV_WARN << "Interface is not connected -- dropping signal " << signal << endl;
        if (!signal->isUpdate() && dynamic_cast<EthernetSignal *>(signal)) { // count only signal starts, do not count JAM and IFG packets
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
                                "maximum allowed length? (%s s corresponds to an approx. %s m cable)",
                    SIMTIME_STR(propagationTime),
                    SIMTIME_STR(propagationTime * SPEED_OF_LIGHT_IN_CABLE));
        }
    }

    EthernetJamSignal *jamSignal = dynamic_cast<EthernetJamSignal *>(signal);

    if (duplexMode && jamSignal)
        throw cRuntimeError("Stray jam signal arrived in full-duplex mode");

    if (dynamic_cast<EthernetJamSignal *>(signal)) {
        updateRxSignals(signal, simTime() + signal->getRemainingDuration());
        processDetectedCollision();
    }
    else
        processMsgFromNetwork(signal);
}

void EthernetCsmaMac::handleUpperPacket(Packet *packet)
{
    EV_INFO << "Received " << packet << " from upper layer." << endl;

    numFramesFromHL++;

    MacAddress address = getMacAddress();

    const auto& macHeader = packet->peekAtFront<EthernetMacHeader>();
    if (macHeader->getDest().equals(address)) {
        throw cRuntimeError("Logic error: frame %s from higher layer has local MAC address as dest (%s)",
                packet->getFullName(), macHeader->getDest().str().c_str());
    }

    if (packet->getDataLength() > MAX_ETHERNET_FRAME_BYTES) {
        throw cRuntimeError("Packet length from higher layer (%s) exceeds maximum Ethernet frame size (%s)",
                packet->getDataLength().str().c_str(), MAX_ETHERNET_FRAME_BYTES.str().c_str());
    }

    if (!connected) {
        EV_WARN << "Interface is not connected -- dropping packet " << packet << endl;
        PacketDropDetails details;
        details.setReason(INTERFACE_DOWN);
        emit(packetDroppedSignal, packet, &details);
        numDroppedPkFromHLIfaceDown++;
        delete packet;

        return;
    }

    if (currentTxFrame != nullptr)
        throw cRuntimeError("EthernetMac already has a transmit packet when packet arrived from upper layer");
    addPaddingAndSetFcs(packet, MIN_ETHERNET_FRAME_BYTES);
    currentTxFrame = packet;
    startFrameTransmission();
}

void EthernetCsmaMac::processMsgFromNetwork(EthernetSignalBase *signal)
{
    simtime_t endRxTime = simTime() + signal->getRemainingDuration();

    if (!duplexMode && receiveState == RX_RECONNECT_STATE) {
        updateRxSignals(signal, endRxTime);
    }
    else if (!duplexMode && (transmitState == TRANSMITTING_STATE || transmitState == SEND_IFG_STATE)) {
        // since we're half-duplex, receiveState must be RX_IDLE_STATE (asserted at top of handleMessage)
        // set receive state and schedule end of reception
        updateRxSignals(signal, endRxTime);
        changeReceptionState(RX_COLLISION_STATE);

        EV_DETAIL << "Transmission interrupted by incoming frame, handling collision\n";
        cancelEvent((transmitState == TRANSMITTING_STATE) ? endTxTimer : endIfgTimer);

        EV_DETAIL << "Transmitting jam signal\n";
        sendJamSignal(); // backoff will be executed when jamming finished

        numCollisions++;
        emit(collisionSignal, 1L);
    }
    else if (receiveState == RX_IDLE_STATE) {
        ASSERT(!signal->isUpdate());
        channelBusySince = simTime();
        EV_INFO << "Reception of " << signal << " started.\n";
        emit(receptionStartedSignal, signal);
        updateRxSignals(signal, endRxTime);
        changeReceptionState(RECEIVING_STATE);
    }
    else if (!signal->isUpdate() && receiveState == RECEIVING_STATE && endRxTimer->getArrivalTime() - simTime() < curEtherDescr->halfBitTime) {
        // With the above condition we filter out "false" collisions that may occur with
        // back-to-back frames. That is: when "beginning of frame" message (this one) occurs
        // BEFORE "end of previous frame" event (endRxMsg) -- same simulation time,
        // only wrong order.

        EV_DETAIL << "Back-to-back frames: completing reception of current frame, starting reception of next one\n";

        // complete reception of previous frame
        cancelEvent(endRxTimer);
        frameReceptionComplete();

        // calculate usability
        totalSuccessfulRxTxTime += simTime() - channelBusySince;
        channelBusySince = simTime();

        // start receiving next frame
        updateRxSignals(signal, endRxTime);
        changeReceptionState(RECEIVING_STATE);
    }
    else { // (receiveState==RECEIVING_STATE || receiveState==RX_COLLISION_STATE)
           // handle overlapping receptions
        // EtherFrame or EtherPauseFrame
        updateRxSignals(signal, endRxTime);
        if (rxSignals.size() > 1) {
            EV_DETAIL << "Overlapping receptions -- setting collision state\n";
            processDetectedCollision();
        }
    }
}

void EthernetCsmaMac::processDetectedCollision()
{
    if (receiveState != RX_COLLISION_STATE) {
        numCollisions++;
        emit(collisionSignal, 1L);
        // go to collision state
        changeReceptionState(RX_COLLISION_STATE);
    }
}

void EthernetCsmaMac::handleEndIFGPeriod()
{
    EV_DETAIL << "IFG elapsed\n";

    if (transmitState == SEND_IFG_STATE) {
        emit(transmissionEndedSignal, curTxSignal);
        txFinished();
        if (canContinueBurst(b(0))) {
            Packet *packet = dequeuePacket();
            handleUpperPacket(packet);
        }
        else
            scheduleEndIFGPeriod();
    }
    else if (transmitState == WAIT_IFG_STATE) {
        // End of IFG period, okay to transmit, if Rx idle OR duplexMode ( checked in startFrameTransmission(); )
        if (currentTxFrame != nullptr)
            startFrameTransmission();
        else if (canDequeuePacket()) {
            Packet *packet = dequeuePacket();
            handleUpperPacket(packet);
        }
        else
            changeTransmissionState(TX_IDLE_STATE);
    }
    else
        throw cRuntimeError("Not in WAIT_IFG_STATE at the end of IFG period");
}

B EthernetCsmaMac::calculateMinFrameLength()
{
    bool inBurst = frameBursting && framesSentInBurst;
    B minFrameLength = duplexMode ? MIN_ETHERNET_FRAME_BYTES : (inBurst ? curEtherDescr->frameInBurstMinBytes : curEtherDescr->halfDuplexFrameMinBytes);

    return minFrameLength;
}

void EthernetCsmaMac::startFrameTransmission()
{
    ASSERT(currentTxFrame);
    EV_DETAIL << "Transmitting a copy of frame " << currentTxFrame << endl;

    Packet *frame = currentTxFrame->dup();

    const auto& hdr = frame->peekAtFront<EthernetMacHeader>();
    ASSERT(hdr);
    ASSERT(!hdr->getSrc().isUnspecified());

    B minFrameLengthWithExtension = calculateMinFrameLength();
    B extensionLength = minFrameLengthWithExtension > frame->getDataLength() ? (minFrameLengthWithExtension - frame->getDataLength()) : B(0);

    // add preamble and SFD (Starting Frame Delimiter), then send out
    encapsulate(frame);

    B sentFrameByteLength = frame->getDataLength() + extensionLength;
    auto& oldPacketProtocolTag = frame->removeTag<PacketProtocolTag>();
    frame->clearTags();
    auto newPacketProtocolTag = frame->addTag<PacketProtocolTag>();
    *newPacketProtocolTag = *oldPacketProtocolTag;
    EV_INFO << "Transmission of " << frame << " started.\n";
    auto signal = new EthernetSignal(frame->getName());
    signal->setSrcMacFullDuplex(duplexMode);
    signal->setBitrate(curEtherDescr->txrate);
    if (sendRawBytes) {
        auto bytes = frame->peekDataAsBytes();
        frame->eraseAll();
        frame->insertAtFront(bytes);
    }
    signal->encapsulate(frame);
    signal->addByteLength(extensionLength.get());
    simtime_t duration = signal->getBitLength() / this->curEtherDescr->txrate;

    sendSignal(signal, duration);

    // check for collisions (there might be an ongoing reception which we don't know about, see below)
    if (!duplexMode && receiveState != RX_IDLE_STATE) {
        // During the IFG period the hardware cannot listen to the channel,
        // so it might happen that receptions have begun during the IFG,
        // and even collisions might be in progress.
        //
        // But we don't know of any ongoing transmission so we blindly
        // start transmitting, immediately collide and send a jam signal.
        //

        processDetectedCollision();
        EV_DETAIL << "startFrameTransmission(): sending JAM signal.\n";
        printState();
        sendJamSignal();
    }
    else {
        // no collision

        // update burst variables
        if (frameBursting) {
            bytesSentInBurst += sentFrameByteLength;
            framesSentInBurst++;
        }

        scheduleAfter(duration, endTxTimer);
        changeTransmissionState(TRANSMITTING_STATE);

        // only count transmissions in totalSuccessfulRxTxTime if channel is half-duplex
        if (!duplexMode)
            channelBusySince = simTime();
    }
}

void EthernetCsmaMac::handleEndTxPeriod()
{
    EV_DETAIL << "TX successfully finished\n";

    // we only get here if transmission has finished successfully, without collision
    if (transmitState != TRANSMITTING_STATE || (!duplexMode && receiveState != RX_IDLE_STATE))
        throw cRuntimeError("End of transmission, and incorrect state detected");

    if (currentTxFrame == nullptr)
        throw cRuntimeError("Frame under transmission cannot be found");

    numFramesSent++;
    numBytesSent += currentTxFrame->getByteLength();
    emit(packetSentToLowerSignal, currentTxFrame); // consider: emit with start time of frame

    emit(transmissionEndedSignal, curTxSignal);
    txFinished();

    const auto& header = currentTxFrame->peekAtFront<EthernetMacHeader>();
    if (header->getTypeOrLength() == ETHERTYPE_FLOW_CONTROL) {
        const auto& controlFrame = currentTxFrame->peekDataAt<EthernetControlFrameBase>(header->getChunkLength(), b(-1));
        if (controlFrame->getOpCode() == ETHERNET_CONTROL_PAUSE) {
            const auto& pauseFrame = dynamicPtrCast<const EthernetPauseFrame>(controlFrame);
            numPauseFramesSent++;
            emit(txPausePkUnitsSignal, pauseFrame->getPauseTime());
        }
    }

    EV_INFO << "Transmission of " << currentTxFrame << " successfully completed.\n";
    deleteCurrentTxFrame();
    lastTxFinishTime = simTime();

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
        if (canContinueBurst(INTERFRAME_GAP_BITS))
            fillIFGInBurst();
        else
            scheduleEndIFGPeriod();
    }
}

void EthernetCsmaMac::handleEndRxPeriod()
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
            for (auto& rx : rxSignals) {
                delete rx.signal;
            }
            rxSignals.clear();
            break;

        case RX_RECONNECT_STATE:
            EV_DETAIL << "Incoming signals finished or reconnect time elapsed after reconnect\n";
            for (auto& rx : rxSignals) {
                delete rx.signal;
            }
            rxSignals.clear();
            break;

        default:
            throw cRuntimeError("model error: invalid receiveState %d", receiveState);
    }

    changeReceptionState(RX_IDLE_STATE);

    if (!duplexMode && transmitState == TX_IDLE_STATE) {
        EV_DETAIL << "Start IFG period\n";
        scheduleEndIFGPeriod();
    }
}

void EthernetCsmaMac::handleEndBackoffPeriod()
{
    if (transmitState != BACKOFF_STATE)
        throw cRuntimeError("At end of BACKOFF and not in BACKOFF_STATE");

    if (currentTxFrame == nullptr)
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

void EthernetCsmaMac::sendSignal(EthernetSignalBase *signal, simtime_t_cref duration)
{
    ASSERT(curTxSignal == nullptr);
    signal->setDuration(duration);
    curTxSignal = signal->dup();
    emit(transmissionStartedSignal, curTxSignal);
    send(signal, SendOptions().transmissionId(curTxSignal->getId()).duration(duration), physOutGate);
}

void EthernetCsmaMac::sendJamSignal()
{
    // abort current transmission
    ASSERT(curTxSignal != nullptr);
    simtime_t duration = simTime() - curTxSignal->getCreationTime(); // TODO save and use start tx time
    cutEthernetSignalEnd(curTxSignal, duration); // TODO save and use start tx time
    emit(transmissionEndedSignal, curTxSignal);
    send(curTxSignal, SendOptions().finishTx(curTxSignal->getId()), physOutGate);
    curTxSignal = nullptr;

    // send JAM
    EthernetJamSignal *jam = new EthernetJamSignal("JAM_SIGNAL");
    jam->setByteLength(B(JAM_SIGNAL_BYTES).get());
    jam->setBitrate(curEtherDescr->txrate);
//    emit(packetSentToLowerSignal, jam);
    duration = jam->getBitLength() / this->curEtherDescr->txrate;
    sendSignal(jam, duration);

    scheduleAfter(duration, endJammingTimer);
    changeTransmissionState(JAMMING_STATE);
}

void EthernetCsmaMac::handleEndJammingPeriod()
{
    if (transmitState != JAMMING_STATE)
        throw cRuntimeError("At end of JAMMING but not in JAMMING_STATE");

    emit(transmissionEndedSignal, curTxSignal);
    txFinished();
    EV_DETAIL << "Jamming finished, executing backoff\n";
    handleRetransmission();
}

void EthernetCsmaMac::handleRetransmission()
{
    if (++backoffs > MAX_ATTEMPTS) {
        EV_DETAIL << "Number of retransmit attempts of frame exceeds maximum, cancelling transmission of frame\n";
        PacketDropDetails details;
        details.setReason(RETRY_LIMIT_REACHED);
        details.setLimit(MAX_ATTEMPTS);
        dropCurrentTxFrame(details);
        changeTransmissionState(TX_IDLE_STATE);
        backoffs = 0;

        if (canDequeuePacket()) {
            Packet *packet = dequeuePacket();
            handleUpperPacket(packet);
        }
        return;
    }

    int backoffRange = (backoffs >= BACKOFF_RANGE_LIMIT) ? 1024 : (1 << backoffs);
    int slotNumber = intuniform(0, backoffRange - 1);
    EV_DETAIL << "Executing backoff procedure (slotNumber=" << slotNumber << ", backoffRange=[0," << backoffRange - 1 << "]" << endl;

    scheduleAfter(slotNumber * curEtherDescr->slotTime, endBackoffTimer);
    changeTransmissionState(BACKOFF_STATE);
    emit(backoffSlotsGeneratedSignal, slotNumber);

    numBackoffs++;
}

void EthernetCsmaMac::printState()
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
    EV_DETAIL << ",  numConcurrentRxTransmissions: " << rxSignals.size();
    EV_DETAIL << ",  queueLength: " << txQueue->getNumPackets();
    EV_DETAIL << endl;

#undef CASE
}

void EthernetCsmaMac::finish()
{
    EthernetMacBase::finish();

    simtime_t t = simTime();
    simtime_t totalChannelIdleTime = t - totalSuccessfulRxTxTime - totalCollisionTime;
    recordScalar("rx channel idle (%)", 100 * (totalChannelIdleTime / t));
    recordScalar("rx channel utilization (%)", 100 * (totalSuccessfulRxTxTime / t));
    recordScalar("rx channel collision (%)", 100 * (totalCollisionTime / t));
    recordScalar("collisions", numCollisions);
    recordScalar("backoffs", numBackoffs);
}

void EthernetCsmaMac::handleEndPausePeriod()
{
    if (transmitState != PAUSE_STATE)
        throw cRuntimeError("At end of PAUSE and not in PAUSE_STATE");

    EV_DETAIL << "Pause finished, resuming transmissions\n";
    beginSendFrames();
}

void EthernetCsmaMac::frameReceptionComplete()
{
    ASSERT(rxSignals.size() == 1);
    EthernetSignalBase *signal = rxSignals[0].signal;
    rxSignals.clear();

    if (dynamic_cast<EthernetFilledIfgSignal *>(signal) != nullptr) {
        delete signal;
        return;
    }
    if (signal->getSrcMacFullDuplex() != duplexMode)
        throw cRuntimeError("Ethernet misconfiguration: MACs on the same link must be all in full duplex mode, or all in half-duplex mode");

    emit(receptionEndedSignal, signal);

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
    auto macAddressInd = packet->addTag<MacAddressInd>();
    macAddressInd->setSrcAddress(frame->getSrc());
    macAddressInd->setDestAddress(frame->getDest());
    if (dropFrameNotForUs(packet, frame))
        return;

    if (frame->getTypeOrLength() == ETHERTYPE_FLOW_CONTROL) {
        processReceivedControlFrame(packet);
    }
    else {
        EV_INFO << "Reception of " << packet << " successfully completed.\n";
        processReceivedDataFrame(packet);
    }
}

void EthernetCsmaMac::processReceivedDataFrame(Packet *packet)
{
    // statistics
    unsigned long curBytes = packet->getByteLength();
    numFramesReceivedOK++;
    numBytesReceivedOK += curBytes;
    emit(rxPkOkSignal, packet);

    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ethernetMac);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
    if (networkInterface)
        packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(networkInterface->getInterfaceId());

    numFramesPassedToHL++;
    emit(packetSentToUpperSignal, packet);
    // pass up to upper layer
    EV_INFO << "Sending " << packet << " to upper layer.\n";
    send(packet, "upperLayerOut");
}

void EthernetCsmaMac::processReceivedControlFrame(Packet *packet)
{
    packet->popAtFront<EthernetMacHeader>();
    const auto& controlFrame = packet->peekAtFront<EthernetControlFrameBase>();

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
            cancelEvent(endPauseTimer);

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

void EthernetCsmaMac::scheduleEndIFGPeriod()
{
    bytesSentInBurst = B(0);
    framesSentInBurst = 0;
    simtime_t ifgTime = b(INTERFRAME_GAP_BITS).get() / curEtherDescr->txrate;
    scheduleAfter(ifgTime, endIfgTimer);
    changeTransmissionState(WAIT_IFG_STATE);
}

void EthernetCsmaMac::fillIFGInBurst()
{
    EV_TRACE << "fillIFGInBurst(): t=" << simTime() << ", framesSentInBurst=" << framesSentInBurst << ", bytesSentInBurst=" << bytesSentInBurst << endl;

    EthernetFilledIfgSignal *gap = new EthernetFilledIfgSignal("FilledIFG");
    gap->setBitrate(curEtherDescr->txrate);
    bytesSentInBurst += B(gap->getByteLength());
    simtime_t duration = gap->getBitLength() / this->curEtherDescr->txrate;
    sendSignal(gap, duration);
    scheduleAfter(duration, endIfgTimer);
    changeTransmissionState(SEND_IFG_STATE);
}

bool EthernetCsmaMac::canContinueBurst(b remainingGapLength)
{
    if ((frameBursting && framesSentInBurst > 0) && (framesSentInBurst < curEtherDescr->maxFramesInBurst)) {
        if (Packet *pk = txQueue->canPullPacket(gate(upperLayerInGateId)->getPathStartGate())) {
            // TODO before/after FilledIfg!!!
            B pkLength = std::max(MIN_ETHERNET_FRAME_BYTES, B(pk->getDataLength()));
            return (bytesSentInBurst + remainingGapLength + PREAMBLE_BYTES + SFD_BYTES + pkLength) <= curEtherDescr->maxBytesInBurst;
        }
    }
    return false;
}

void EthernetCsmaMac::scheduleEndPausePeriod(int pauseUnits)
{
    // length is interpreted as 512-bit-time units
    simtime_t pausePeriod = pauseUnits * PAUSE_UNIT_BITS / curEtherDescr->txrate;
    scheduleAfter(pausePeriod, endPauseTimer);
    changeTransmissionState(PAUSE_STATE);
}

void EthernetCsmaMac::beginSendFrames()
{
    if (currentTxFrame) {
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

void EthernetCsmaMac::updateRxSignals(EthernetSignalBase *signal, simtime_t endRxTime)
{
    simtime_t maxEndRxTime = endRxTime;
    bool found = false;
    bool isUpdate = signal->isUpdate();
    long signalTransmissionId = signal->getTransmissionId();

    for (auto& rx : rxSignals) {
        if (isUpdate && rx.transmissionId == signalTransmissionId) {
            ASSERT(!found);
            found = true;
            delete rx.signal;
            rx.signal = signal;
            rx.endRxTime = endRxTime;
        }

        if (rx.endRxTime > maxEndRxTime)
            maxEndRxTime = rx.endRxTime;
    }

    if (!found)
        rxSignals.push_back(RxSignal(signalTransmissionId, signal, endRxTime));

    if (endRxTimer->getArrivalTime() != maxEndRxTime || endRxTime == maxEndRxTime) {
        rescheduleAt(maxEndRxTime, endRxTimer);
    }
}

void EthernetCsmaMac::dropCurrentTxFrame(PacketDropDetails& details)
{
    EthernetMacBase::dropCurrentTxFrame(details);
    delete curTxSignal;
    curTxSignal = nullptr;
}

void EthernetCsmaMac::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (duplexMode || receiveState == RX_IDLE_STATE) {
        if (currentTxFrame == nullptr && transmitState == TX_IDLE_STATE && canDequeuePacket()) {
            Packet *packet = dequeuePacket();
            handleUpperPacket(packet);
        }
    }
}

} // namespace inet

