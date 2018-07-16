//
// Copyright (C) 2006 Levente Meszaros
// Copyright (C) 2011 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/linklayer/ethernet/EtherMacFullDuplex.h"

#include "inet/common/queue/IPassiveQueue.h"
#include "inet/common/Simsignals.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ethernet/EtherEncap.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

// TODO: refactor using a statemachine that is present in a single function
// TODO: this helps understanding what interactions are there and how they affect the state

Define_Module(EtherMacFullDuplex);

EtherMacFullDuplex::EtherMacFullDuplex()
{
}

void EtherMacFullDuplex::initialize(int stage)
{
    EtherMacBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        if (!par("duplexMode"))
            throw cRuntimeError("Half duplex operation is not supported by EtherMacFullDuplex, use the EtherMac module for that! (Please enable csmacdSupport on EthernetInterface)");
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        beginSendFrames();    //FIXME choose an another stage for it
    }
}

void EtherMacFullDuplex::initializeStatistics()
{
    EtherMacBase::initializeStatistics();

    // initialize statistics
    totalSuccessfulRxTime = 0.0;
}

void EtherMacFullDuplex::initializeFlags()
{
    EtherMacBase::initializeFlags();

    duplexMode = true;
    physInGate->setDeliverOnReceptionStart(false);
}

void EtherMacFullDuplex::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        handleMessageWhenDown(msg);
        return;
    }

    if (channelsDiffer)
        readChannelParameters(true);

    if (msg->isSelfMessage())
        handleSelfMessage(msg);
    else if (msg->getArrivalGate() == upperLayerInGate)
        processFrameFromUpperLayer(check_and_cast<Packet *>(msg));
    else if (msg->getArrivalGate() == physInGate)
        processMsgFromNetwork(check_and_cast<EthernetSignal *>(msg));
    else
        throw cRuntimeError("Message received from unknown gate!");
}

void EtherMacFullDuplex::handleSelfMessage(cMessage *msg)
{
    EV_TRACE << "Self-message " << msg << " received\n";

    if (msg == endTxMsg)
        handleEndTxPeriod();
    else if (msg == endIFGMsg)
        handleEndIFGPeriod();
    else if (msg == endPauseMsg)
        handleEndPausePeriod();
    else
        throw cRuntimeError("Unknown self message received!");
}

void EtherMacFullDuplex::startFrameTransmission()
{
    ASSERT(curTxFrame);
    EV_DETAIL << "Transmitting a copy of frame " << curTxFrame << endl;

    Packet *frame = curTxFrame->dup();    // note: we need to duplicate the frame because we emit a signal with it in endTxPeriod()
    const auto& hdr = frame->peekAtFront<EthernetMacHeader>();    // note: we need to duplicate the frame because we emit a signal with it in endTxPeriod()
    ASSERT(hdr);
    ASSERT(!hdr->getSrc().isUnspecified());

    if (frame->getDataLength() < curEtherDescr->frameMinBytes) {
        auto oldFcs = frame->removeAtBack<EthernetFcs>();
        EtherEncap::addPaddingAndFcs(frame, oldFcs->getFcsMode(), curEtherDescr->frameMinBytes);
    }

    // add preamble and SFD (Starting Frame Delimiter), then send out
    encapsulate(frame);

    // send
    EV_INFO << "Transmission of " << frame << " started.\n";
    auto oldPacketProtocolTag = frame->removeTag<PacketProtocolTag>();
    frame->clearTags();
    auto newPacketProtocolTag = frame->addTag<PacketProtocolTag>();
    *newPacketProtocolTag = *oldPacketProtocolTag;
    delete oldPacketProtocolTag;
    auto signal = new EthernetSignal(frame->getName());
    if (sendRawBytes) {
        signal->encapsulate(new Packet(frame->getName(), frame->peekAllAsBytes()));
        delete frame;
    }
    else
        signal->encapsulate(frame);
    send(signal, physOutGate);

    scheduleAt(transmissionChannel->getTransmissionFinishTime(), endTxMsg);
    changeTransmissionState(TRANSMITTING_STATE);
}

void EtherMacFullDuplex::processFrameFromUpperLayer(Packet *packet)
{
    ASSERT(packet->getDataLength() >= MIN_ETHERNET_FRAME_BYTES);

    EV_INFO << "Received " << packet << " from upper layer." << endl;

    numFramesFromHL++;
    emit(packetReceivedFromUpperSignal, packet);

    auto frame = packet->peekAtFront<EthernetMacHeader>();
    if (frame->getDest().equals(address)) {
        throw cRuntimeError("logic error: frame %s from higher layer has local MAC address as dest (%s)",
                packet->getFullName(), frame->getDest().str().c_str());
    }

    if (packet->getDataLength() > MAX_ETHERNET_FRAME_BYTES) {    //FIXME two MAX FRAME BYTES in specif...
        throw cRuntimeError("packet from higher layer (%d bytes) exceeds maximum Ethernet frame size (%d)",
                (int)(packet->getByteLength()), B(MAX_ETHERNET_FRAME_BYTES).get());
    }

    if (!connected || disabled) {
        EV_WARN << (!connected ? "Interface is not connected" : "MAC is disabled") << " -- dropping packet " << packet << endl;
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
        frame = nullptr; // drop shared ptr
        auto newFrame = packet->removeAtFront<EthernetMacHeader>();
        newFrame->setSrc(address);
        packet->insertAtFront(newFrame);
        frame = newFrame;
        auto oldFcs = packet->removeAtBack<EthernetFcs>();
        EtherEncap::addFcs(packet, oldFcs->getFcsMode());
    }

    if (txQueue.extQueue) {
        ASSERT(curTxFrame == nullptr);
        ASSERT(transmitState == TX_IDLE_STATE || transmitState == PAUSE_STATE);
        curTxFrame = packet;
    }
    else {
        if (txQueue.innerQueue->isFull())
            throw cRuntimeError("txQueue length exceeds %d -- this is probably due to "
                                "a bogus app model generating excessive traffic "
                                "(or if this is normal, increase txQueueLimit!)",
                    txQueue.innerQueue->getQueueLimit());
        // store frame and possibly begin transmitting
        EV_DETAIL << "Frame " << frame << " arrived from higher layers, enqueueing\n";
        txQueue.innerQueue->insertFrame(packet);

        if (!curTxFrame && !txQueue.innerQueue->isEmpty() && transmitState == TX_IDLE_STATE)
            curTxFrame = check_and_cast<Packet *>(txQueue.innerQueue->pop());
    }

    if (transmitState == TX_IDLE_STATE)
        startFrameTransmission();
}

void EtherMacFullDuplex::processMsgFromNetwork(EthernetSignal *signal)
{
    EV_INFO << signal << " received." << endl;

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

    if (dynamic_cast<EthernetFilledIfgSignal *>(signal))
        throw cRuntimeError("There is no burst mode in full-duplex operation: EtherFilledIfg is unexpected");
    bool hasBitError = signal->hasBitError();
    auto packet = check_and_cast<Packet *>(signal->decapsulate());
    delete signal;
    totalSuccessfulRxTime += packet->getDuration();
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
        const auto& controlFrame = curTxFrame->peekDataAt<EthernetControlFrame>(frame->getChunkLength(), b(-1));
        if (controlFrame->getOpCode() == ETHERNET_CONTROL_PAUSE) {
            auto pauseFrame = check_and_cast<const EthernetPauseFrame *>(controlFrame.get());
            int pauseUnits = pauseFrame->getPauseTime();
            delete packet;
            numPauseFramesRcvd++;
            emit(rxPausePkUnitsSignal, pauseUnits);
            processPauseCommand(pauseUnits);
        }
        else {
            EV_INFO << "Received unknown ethernet flow control frame" << frame << " dropped." << endl;
            delete packet;
        }
    }
    else {
        EV_INFO << "Reception of " << frame << " successfully completed." << endl;
        processReceivedDataFrame(packet, frame);
    }
}

void EtherMacFullDuplex::handleEndIFGPeriod()
{
    ASSERT(nullptr == curTxFrame);
    if (transmitState != WAIT_IFG_STATE)
        throw cRuntimeError("Not in WAIT_IFG_STATE at the end of IFG period");

    // End of IFG period, okay to transmit
    EV_DETAIL << "IFG elapsed" << endl;

    getNextFrameFromQueue();
    beginSendFrames();
}

void EtherMacFullDuplex::handleEndTxPeriod()
{
    // we only get here if transmission has finished successfully
    if (transmitState != TRANSMITTING_STATE)
        throw cRuntimeError("Model error: End of transmission, and incorrect state detected");

    if (nullptr == curTxFrame)
        throw cRuntimeError("Model error: Frame under transmission cannot be found");

    numFramesSent++;
    numBytesSent += curTxFrame->getByteLength();
    emit(packetSentToLowerSignal, curTxFrame);    //consider: emit with start time of frame

    const auto& header = curTxFrame->peekAtFront<EthernetMacHeader>();
    if (header->getTypeOrLength() == ETHERTYPE_FLOW_CONTROL) {
        const auto& controlFrame = curTxFrame->peekDataAt<EthernetControlFrame>(header->getChunkLength(), b(-1));
        if (controlFrame->getOpCode() == ETHERNET_CONTROL_PAUSE) {
            const auto& pauseFrame = CHK(dynamicPtrCast<const EthernetPauseFrame>(controlFrame));
            numPauseFramesSent++;
            emit(txPausePkUnitsSignal, pauseFrame->getPauseTime());
        }
    }

    EV_INFO << "Transmission of " << curTxFrame << " successfully completed.\n";
    delete curTxFrame;
    curTxFrame = nullptr;
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

void EtherMacFullDuplex::finish()
{
    EtherMacBase::finish();

    simtime_t t = simTime();
    simtime_t totalRxChannelIdleTime = t - totalSuccessfulRxTime;
    recordScalar("rx channel idle (%)", 100 * (totalRxChannelIdleTime / t));
    recordScalar("rx channel utilization (%)", 100 * (totalSuccessfulRxTime / t));
}

void EtherMacFullDuplex::handleEndPausePeriod()
{
    ASSERT(nullptr == curTxFrame);
    if (transmitState != PAUSE_STATE)
        throw cRuntimeError("End of PAUSE event occurred when not in PAUSE_STATE!");

    EV_DETAIL << "Pause finished, resuming transmissions\n";
    getNextFrameFromQueue();
    beginSendFrames();
}

void EtherMacFullDuplex::processReceivedDataFrame(Packet *packet, const Ptr<const EthernetMacHeader>& frame)
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

void EtherMacFullDuplex::processPauseCommand(int pauseUnits)
{
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

void EtherMacFullDuplex::scheduleEndIFGPeriod()
{
    ASSERT(nullptr == curTxFrame);
    changeTransmissionState(WAIT_IFG_STATE);
    simtime_t endIFGTime = simTime() + (b(INTERFRAME_GAP_BITS).get() / curEtherDescr->txrate);
    scheduleAt(endIFGTime, endIFGMsg);
}

void EtherMacFullDuplex::scheduleEndPausePeriod(int pauseUnits)
{
    ASSERT(nullptr == curTxFrame);
    // length is interpreted as 512-bit-time units
    simtime_t pausePeriod = ((pauseUnits * PAUSE_UNIT_BITS) / curEtherDescr->txrate);
    scheduleAt(simTime() + pausePeriod, endPauseMsg);
    changeTransmissionState(PAUSE_STATE);
}

void EtherMacFullDuplex::beginSendFrames()
{
    if (curTxFrame) {
        // Other frames are queued, transmit next frame
        EV_DETAIL << "Transmit next frame in output queue\n";
        startFrameTransmission();
    }
    else {
        // No more frames set transmitter to idle
        changeTransmissionState(TX_IDLE_STATE);
        if (!txQueue.extQueue) {
            // Output only for internal queue (we cannot be shure that there
            //are no other frames in external queue)
            EV_DETAIL << "No more frames to send, transmitter set to idle\n";
        }
    }
}

} // namespace inet

