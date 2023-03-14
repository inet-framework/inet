//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/basic/EthernetMac.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/common/EthernetControlFrame_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wired/ethernet/EthernetSignal_m.h"

namespace inet {

// TODO refactor using a statemachine that is present in a single function
// TODO this helps understanding what interactions are there and how they affect the state

Define_Module(EthernetMac);

EthernetMac::EthernetMac()
{
}

void EthernetMac::initialize(int stage)
{
    EthernetMacBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        if (!par("duplexMode"))
            throw cRuntimeError("Half duplex operation is not supported by EthernetMac, use the EthernetCsmaMac module for that! (Please enable csmacdSupport on EthernetInterface)");
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        beginSendFrames(); // FIXME choose an another stage for it
    }
}

void EthernetMac::initializeStatistics()
{
    EthernetMacBase::initializeStatistics();

    // initialize statistics
    totalSuccessfulRxTime = 0.0;
}

void EthernetMac::initializeFlags()
{
    EthernetMacBase::initializeFlags();

    duplexMode = true;
    physInGate->setDeliverImmediately(false);
}

void EthernetMac::handleMessageWhenUp(cMessage *msg)
{
    if (channelsDiffer)
        readChannelParameters(true);

    if (msg->isSelfMessage())
        handleSelfMessage(msg);
    else if (msg->getArrivalGateId() == upperLayerInGateId)
        handleUpperPacket(check_and_cast<Packet *>(msg));
    else if (msg->getArrivalGate() == physInGate)
        processMsgFromNetwork(check_and_cast<Signal *>(msg));
    else
        throw cRuntimeError("Message received from unknown gate!");
    processAtHandleMessageFinished();
}

void EthernetMac::handleSelfMessage(cMessage *msg)
{
    EV_TRACE << "Self-message " << msg << " received\n";

    if (msg == endTxTimer)
        handleEndTxPeriod();
    else if (msg == endIfgTimer)
        handleEndIFGPeriod();
    else if (msg == endPauseTimer)
        handleEndPausePeriod();
    else
        throw cRuntimeError("Unknown self message received!");
}

void EthernetMac::startFrameTransmission()
{
    ASSERT(currentTxFrame);
    EV_DETAIL << "Transmitting a copy of frame " << currentTxFrame << endl;

    Packet *frame = currentTxFrame->dup(); // note: we need to duplicate the frame because we emit a signal with it in endTxPeriod()
    const auto& hdr = frame->peekAtFront<EthernetMacHeader>(); // note: we need to duplicate the frame because we emit a signal with it in endTxPeriod()
    ASSERT(hdr);
    ASSERT(!hdr->getSrc().isUnspecified());

    // add preamble and SFD (Starting Frame Delimiter), then send out
    encapsulate(frame);

    // send
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
    ASSERT(curTxSignal == nullptr);
    curTxSignal = signal->dup();
    emit(transmissionStartedSignal, signal);
    send(signal, SendOptions().transmissionId(curTxSignal->getId()), physOutGate);
    scheduleAt(transmissionChannel->getTransmissionFinishTime(), endTxTimer);
    changeTransmissionState(TRANSMITTING_STATE);
}

void EthernetMac::handleUpperPacket(Packet *packet)
{
    //ASSERT2(false, "using EthernetMac");
    EV_INFO << "Received " << packet << " from upper layer." << endl;

    numFramesFromHL++;

    const auto& frame = packet->peekAtFront<EthernetMacHeader>();
    if (frame->getDest().equals(getMacAddress())) {
        throw cRuntimeError("logic error: frame %s from higher layer has local MAC address as dest (%s)",
                packet->getFullName(), frame->getDest().str().c_str());
    }

    if (packet->getDataLength() > MAX_ETHERNET_FRAME_BYTES) { // FIXME two MAX FRAME BYTES in specif...
        throw cRuntimeError("packet from higher layer (%s) exceeds maximum Ethernet frame size (%s)",
                B(packet->getByteLength()).str().c_str(), MAX_ETHERNET_FRAME_BYTES.str().c_str());
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

    if (transmitState != TX_IDLE_STATE)
        throw cRuntimeError("EthernetMac not in TX_IDLE_STATE when packet arrived from upper layer");
    if (currentTxFrame != nullptr)
        throw cRuntimeError("EthernetMac already has a transmit packet when packet arrived from upper layer");
    addPaddingAndSetFcs(packet, MIN_ETHERNET_FRAME_BYTES);
    currentTxFrame = packet;
    startFrameTransmission();
}

void EthernetMac::processMsgFromNetwork(Signal *signal)
{
    EV_INFO << signal << " received." << endl;

    if (!connected) {
        EV_WARN << "Interface is not connected -- dropping signal " << signal << endl;
        if (dynamic_cast<EthernetSignal *>(signal)) { // do not count JAM and IFG packets
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

    emit(receptionEndedSignal, signal);

    totalSuccessfulRxTime += signal->getDuration();

    if (auto ethernetSignal = dynamic_cast<EthernetSignalBase *>(signal))
        if (ethernetSignal->getSrcMacFullDuplex() != duplexMode)
            throw cRuntimeError("Ethernet misconfiguration: MACs on the same link must be all in full duplex mode, or all in half-duplex mode");

    if (dynamic_cast<EthernetFilledIfgSignal *>(signal))
        throw cRuntimeError("There is no burst mode in full-duplex operation: EtherFilledIfg is unexpected");

    if (dynamic_cast<EthernetJamSignal *>(signal))
        throw cRuntimeError("There is no JAM signal in full-duplex operation: EthernetJamSignal is unexpected");

    bool hasBitError = signal->hasBitError();
    auto packet = check_and_cast<Packet *>(signal->decapsulate());
    delete signal;
    decapsulate(packet);
    emit(packetReceivedFromLowerSignal, packet);

    if (hasBitError || !verifyCrcAndLength(packet)) {
        EV_WARN << "Bit error/CRC error in incoming packet -- dropping packet " << EV_FIELD(packet) << EV_ENDL;
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
        const auto& controlFrame = packet->peekDataAt<EthernetControlFrameBase>(frame->getChunkLength(), b(-1));
        if (controlFrame->getOpCode() == ETHERNET_CONTROL_PAUSE) {
            auto pauseFrame = check_and_cast<const EthernetPauseFrame *>(controlFrame.get());
            int pauseUnits = pauseFrame->getPauseTime();
            EV_INFO << "Reception of PAUSE frame " << packet << " successfully completed." << endl;
            delete packet;
            numPauseFramesRcvd++;
            emit(rxPausePkUnitsSignal, pauseUnits);
            processPauseCommand(pauseUnits);
        }
        else {
            EV_INFO << "Received unknown ethernet flow control frame " << EV_FIELD(packet) << ", dropped." << endl;
            delete packet;
        }
    }
    else {
        EV_INFO << "Reception of " << EV_FIELD(packet) << " successfully completed." << endl;
        processReceivedDataFrame(packet, frame);
    }
}

void EthernetMac::handleEndIFGPeriod()
{
    ASSERT(nullptr == currentTxFrame);
    if (transmitState != WAIT_IFG_STATE)
        throw cRuntimeError("Not in WAIT_IFG_STATE at the end of IFG period");

    // End of IFG period, okay to transmit
    EV_DETAIL << "IFG elapsed" << endl;
    changeTransmissionState(TX_IDLE_STATE);

    if (canDequeuePacket()) {
        Packet *packet = dequeuePacket();
        handleUpperPacket(packet);
    }
}

void EthernetMac::handleEndTxPeriod()
{
    // we only get here if transmission has finished successfully
    if (transmitState != TRANSMITTING_STATE)
        throw cRuntimeError("Model error: End of transmission, and incorrect state detected");

    if (nullptr == currentTxFrame)
        throw cRuntimeError("Model error: Frame under transmission cannot be found");

    numFramesSent++;
    numBytesSent += currentTxFrame->getByteLength();
    emit(packetSentToLowerSignal, currentTxFrame); // consider: emit with start time of frame

    emit(transmissionEndedSignal, curTxSignal);
    txFinished();

    const auto& header = currentTxFrame->peekAtFront<EthernetMacHeader>();
    if (header->getTypeOrLength() == ETHERTYPE_FLOW_CONTROL) {
        const auto& controlFrame = currentTxFrame->peekDataAt<EthernetControlFrameBase>(header->getChunkLength(), b(-1));
        if (controlFrame->getOpCode() == ETHERNET_CONTROL_PAUSE) {
            const auto& pauseFrame = CHK(dynamicPtrCast<const EthernetPauseFrame>(controlFrame));
            numPauseFramesSent++;
            emit(txPausePkUnitsSignal, pauseFrame->getPauseTime());
        }
    }

    EV_INFO << "Transmission of " << currentTxFrame << " successfully completed.\n";
    deleteCurrentTxFrame();
    lastTxFinishTime = simTime();

    if (pauseUnitsRequested > 0) {
        // if we received a PAUSE frame recently, go into PAUSE state
        EV_DETAIL << "Going to PAUSE mode for " << pauseUnitsRequested << " time units\n";

        scheduleEndPausePeriod(pauseUnitsRequested);
        pauseUnitsRequested = 0;
    }
    else {
        EV_DETAIL << "Start IFG period\n";
        scheduleEndIFGPeriod();
    }
}

void EthernetMac::finish()
{
    EthernetMacBase::finish();

    simtime_t t = simTime();
    simtime_t totalRxChannelIdleTime = t - totalSuccessfulRxTime;
    recordScalar("rx channel idle (%)", 100 * (totalRxChannelIdleTime / t));
    recordScalar("rx channel utilization (%)", 100 * (totalSuccessfulRxTime / t));
}

void EthernetMac::handleEndPausePeriod()
{
    ASSERT(nullptr == currentTxFrame);
    if (transmitState != PAUSE_STATE)
        throw cRuntimeError("End of PAUSE event occurred when not in PAUSE_STATE!");

    EV_DETAIL << "Pause finished, resuming transmissions\n";
    changeTransmissionState(TX_IDLE_STATE);

    if (canDequeuePacket()) {
        Packet *packet = dequeuePacket();
        handleUpperPacket(packet);
    }
}

void EthernetMac::processReceivedDataFrame(Packet *packet, const Ptr<const EthernetMacHeader>& frame)
{
    // statistics
    unsigned long curBytes = packet->getByteLength();
    numFramesReceivedOK++;
    numBytesReceivedOK += curBytes;
    emit(rxPkOkSignal, packet);

    const auto macAddressInd = packet->addTagIfAbsent<MacAddressInd>();
    macAddressInd->setSrcAddress(frame->getSrc());
    macAddressInd->setDestAddress(frame->getDest());
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ethernetMac);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
    if (networkInterface)
        packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(networkInterface->getInterfaceId());

    numFramesPassedToHL++;
    emit(packetSentToUpperSignal, packet);
    // pass up to upper layer
    EV_INFO << "Sending " << packet << " to upper layer.\n";
    send(packet, upperLayerOutGateId);
}

void EthernetMac::processPauseCommand(int pauseUnits)
{
    if (transmitState == TX_IDLE_STATE) {
        EV_DETAIL << "PAUSE frame received, pausing for " << pauseUnitsRequested << " time units\n";
        if (pauseUnits > 0)
            scheduleEndPausePeriod(pauseUnits);
    }
    else if (transmitState == PAUSE_STATE) {
        EV_DETAIL << "PAUSE frame received, pausing for " << pauseUnitsRequested
                  << " more time units from now\n";
        cancelEvent(endPauseTimer);

        // Terminate PAUSE if pauseUnits == 0; Extend PAUSE if pauseUnits > 0
        scheduleEndPausePeriod(pauseUnits);
    }
    else {
        // transmitter busy -- wait until it finishes with current frame (endTx)
        // and then it'll go to PAUSE state
        EV_DETAIL << "PAUSE frame received, storing pause request\n";
        pauseUnitsRequested = pauseUnits;
    }
}

void EthernetMac::scheduleEndIFGPeriod()
{
    ASSERT(nullptr == currentTxFrame);
    changeTransmissionState(WAIT_IFG_STATE);
    simtime_t endIFGTime = simTime() + (b(INTERFRAME_GAP_BITS).get() / curEtherDescr->txrate);
    scheduleAt(endIFGTime, endIfgTimer);
}

void EthernetMac::scheduleEndPausePeriod(int pauseUnits)
{
    ASSERT(nullptr == currentTxFrame);
    // length is interpreted as 512-bit-time units
    simtime_t pausePeriod = ((pauseUnits * PAUSE_UNIT_BITS) / curEtherDescr->txrate);
    scheduleAfter(pausePeriod, endPauseTimer);
    changeTransmissionState(PAUSE_STATE);
}

void EthernetMac::beginSendFrames()
{
    if (currentTxFrame) {
        // Other frames are queued, transmit next frame
        EV_DETAIL << "Transmit next frame in output queue\n";
        startFrameTransmission();
    }
    else {
        // No more frames set transmitter to idle
        changeTransmissionState(TX_IDLE_STATE);
        EV_DETAIL << "No more frames to send, transmitter set to idle\n";
    }
}

void EthernetMac::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (currentTxFrame == nullptr && transmitState == TX_IDLE_STATE && canDequeuePacket()) {
        Packet *packet = dequeuePacket();
        handleUpperPacket(packet);
    }
}

} // namespace inet

