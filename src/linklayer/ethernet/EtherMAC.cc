/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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

#include "EtherMAC.h"

#include "EtherFrame_m.h"
#include "Ethernet.h"
#include "Ieee802Ctrl_m.h"
#include "IPassiveQueue.h"



static std::ostream& operator<<(std::ostream& out, cMessage *msg)
{
    out << "(" << msg->getClassName() << ")" << msg->getFullName();
    return out;
}


Define_Module( EtherMAC );

simsignal_t EtherMAC::collisionSignal = SIMSIGNAL_NULL;
simsignal_t EtherMAC::backoffSignal = SIMSIGNAL_NULL;

EtherMAC::EtherMAC()
{
    frameBeingReceived = NULL;
    endJammingMsg = endRxMsg = endBackoffMsg = NULL;
}

EtherMAC::~EtherMAC()
{
    delete frameBeingReceived;
    cancelAndDelete(endRxMsg);
    cancelAndDelete(endBackoffMsg);
    cancelAndDelete(endJammingMsg);
}

void EtherMAC::initialize()
{
    EtherMACBase::initialize();

    endRxMsg = new cMessage("EndReception", ENDRECEPTION);
    endBackoffMsg = new cMessage("EndBackoff", ENDBACKOFF);
    endJammingMsg = new cMessage("EndJamming", ENDJAMMING);

    // initialize state info
    backoffs = 0;
    numConcurrentTransmissions = 0;

    WATCH(backoffs);
    WATCH(numConcurrentTransmissions);
}

void EtherMAC::initializeStatistics()
{
    EtherMACBase::initializeStatistics();

    // initialize statistics
    totalCollisionTime = 0.0;
    totalSuccessfulRxTxTime = 0.0;
    numCollisions = numBackoffs = 0;

    WATCH(numCollisions);
    WATCH(numBackoffs);

    collisionSignal = registerSignal("collision");
    backoffSignal = registerSignal("backoff");
}

void EtherMAC::initializeFlags()
{
    EtherMACBase::initializeFlags();

    duplexMode = par("duplexEnabled");
    physInGate->setDeliverOnReceptionStart(true);
}

void EtherMAC::handleMessage(cMessage *msg)
{
    printState();

    // some consistency check
    if (!duplexMode && transmitState == TRANSMITTING_STATE && receiveState != RX_IDLE_STATE)
        error("Inconsistent state -- transmitting and receiving at the same time");

    if (!msg->isSelfMessage())
    {
        // either frame from upper layer, or frame/jam signal from the network
        if (!connected)
            processMessageWhenNotConnected(msg);
        else if (disabled)
            processMessageWhenDisabled(msg);
        else if (msg->getArrivalGate() == gate("upperLayerIn"))
            processFrameFromUpperLayer(check_and_cast<EtherFrame *>(msg));
        else
            processMsgFromNetwork(check_and_cast<EtherTraffic *>(msg));
    }
    else
    {
        // Process different self-messages (timer signals)
        EV << "Self-message " << msg << " received\n";

        switch (msg->getKind())
        {
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
                error("self-message with unexpected message kind %d", msg->getKind());
        }
    }
    printState();

    if (ev.isGUI())
        updateDisplayString();
}


void EtherMAC::processFrameFromUpperLayer(EtherFrame *frame)
{
    EV << "Received frame from upper layer: " << frame << endl;

    emit(packetReceivedFromUpperSignal, frame);

    if (frame->getDest().equals(address))
    {
        error("logic error: frame %s from higher layer has local MAC address as dest (%s)",
                frame->getFullName(), frame->getDest().str().c_str());
    }

    if (frame->getByteLength() > MAX_ETHERNET_FRAME)
    {
        error("packet from higher layer (%d bytes) exceeds maximum Ethernet frame size (%d)",
                (int)(frame->getByteLength()), MAX_ETHERNET_FRAME);
    }

    // fill in src address if not set
    if (frame->getSrc().isUnspecified())
        frame->setSrc(address);

    bool isPauseFrame = (dynamic_cast<EtherPauseFrame*>(frame) != NULL);

    if (!isPauseFrame)
    {
        numFramesFromHL++;
        emit(rxPkBytesFromHLSignal, (long)(frame->getByteLength()));
    }

    if (txQueue.extQueue)
    {
        ASSERT(curTxFrame == NULL);
        curTxFrame = frame;
    }
    else
    {
        if (!isPauseFrame)
        {
            if (txQueue.innerQueue->queueLimit
                    && txQueue.innerQueue->queue.length() > txQueue.innerQueue->queueLimit)
            {
                error("txQueue length exceeds %d -- this is probably due to "
                      "a bogus app model generating excessive traffic "
                      "(or if this is normal, increase txQueueLimit!)",
                      txQueue.innerQueue->queueLimit);
            }

            // store frame and possibly begin transmitting
            EV << "Packet " << frame << " arrived from higher layers, enqueueing\n";
            txQueue.innerQueue->queue.insert(frame);
        }
        else
        {
            EV << "PAUSE received from higher layer\n";

            // PAUSE frames enjoy priority -- they're transmitted before all other frames queued up
            if (!txQueue.innerQueue->queue.empty())
                // front() frame is probably being transmitted
                txQueue.innerQueue->queue.insertBefore(txQueue.innerQueue->queue.front(), frame);
            else
                txQueue.innerQueue->queue.insert(frame);
        }

        if (NULL == curTxFrame && !txQueue.innerQueue->queue.empty())
            curTxFrame = (EtherFrame*)txQueue.innerQueue->queue.pop();
    }

    if ((duplexMode || receiveState == RX_IDLE_STATE) && transmitState == TX_IDLE_STATE)
    {
        EV << "No incoming carrier signals detected, frame clear to send, wait IFG first\n";
        scheduleEndIFGPeriod();
    }
}


void EtherMAC::processMsgFromNetwork(EtherTraffic *msg)
{
    EV << "Received frame from network: " << msg << endl;

    // detect cable length violation in half-duplex mode
    if (!duplexMode)
    {
        if (simTime() - msg->getSendingTime() >= curEtherDescr->shortestFrameDuration)
        {
            error("very long frame propagation time detected, "
                  "maybe cable exceeds maximum allowed length? "
                  "(%lgs corresponds to an approx. %lgm cable)",
                  SIMTIME_STR(simTime() - msg->getSendingTime()),
                  SIMTIME_STR((simTime() - msg->getSendingTime()) * SPEED_OF_LIGHT_IN_CABLE));
        }
    }

//TODO handling received EtherIFG

    simtime_t endRxTime = simTime() + msg->getDuration();

    if (!duplexMode && (transmitState == TRANSMITTING_STATE || transmitState == SEND_IFG_STATE))
    {
        // since we're halfduplex, receiveState must be RX_IDLE_STATE (asserted at top of handleMessage)
        if (dynamic_cast<EtherJam*>(msg) != NULL)
            error("Stray jam signal arrived while transmitting (usual cause is cable length exceeding allowed maximum)");

        EV << "Transmission interrupted by incoming frame, handling collision\n";
        cancelEvent((transmitState==TRANSMITTING_STATE) ? endTxMsg : endIFGMsg);

        EV << "Transmitting jam signal\n";
        sendJamSignal(); // backoff will be executed when jamming finished

        // set receive state and schedule end of reception
        receiveState = RX_COLLISION_STATE;
        numConcurrentTransmissions++;

        cPacket jam;
        jam.setByteLength(JAM_SIGNAL_BYTES);
        simtime_t endJamTime = simTime() + transmissionChannel->calculateDuration(&jam);
        scheduleAt(endRxTime < endJamTime ? endJamTime : endRxTime, endRxMsg);
        delete msg;

        numCollisions++;
        emit(collisionSignal, 1L);
    }
    else if (receiveState == RX_IDLE_STATE)
    {
        if (dynamic_cast<EtherJam*>(msg) != NULL)
            error("Stray jam signal arrived (usual cause is cable length exceeding allowed maximum)");

        EV << "Start reception of frame\n";
        numConcurrentTransmissions++;

        if (frameBeingReceived)
            error("frameBeingReceived!=0 in RX_IDLE_STATE");

        frameBeingReceived = (EtherFrame *)msg;
        scheduleEndRxPeriod(msg);
        channelBusySince = simTime();
    }
    else if (receiveState == RECEIVING_STATE
            && dynamic_cast<EtherJam*>(msg) == NULL
            && endRxMsg->getArrivalTime() - simTime() < curEtherDescr->halfBitTime)
    {
        // With the above condition we filter out "false" collisions that may occur with
        // back-to-back frames. That is: when "beginning of frame" message (this one) occurs
        // BEFORE "end of previous frame" event (endRxMsg) -- same simulation time,
        // only wrong order.

        EV << "Back-to-back frames: completing reception of current frame, starting reception of next one\n";

        // complete reception of previous frame
        cancelEvent(endRxMsg);
        EtherTraffic *frame = frameBeingReceived;
        frameBeingReceived = NULL;
        frameReceptionComplete(frame);

        // calculate usability
        totalSuccessfulRxTxTime += simTime()-channelBusySince;
        channelBusySince = simTime();

        // start receiving next frame
        frameBeingReceived = (EtherTraffic *)msg;
        scheduleEndRxPeriod(msg);
    }
    else // (receiveState==RECEIVING_STATE || receiveState==RX_COLLISION_STATE)
    {
        // handle overlapping receptions
        if (dynamic_cast<EtherJam*>(msg) != NULL)
        {
            if (numConcurrentTransmissions <= 0)
                error("numConcurrentTransmissions=%d on jam arrival (stray jam?)", numConcurrentTransmissions);

            numConcurrentTransmissions--;
            EV << "Jam signal received, this marks end of one transmission\n";

            // by the time jamming ends, all transmissions will have been aborted
            if (numConcurrentTransmissions == 0)
            {
                EV << "Last jam signal received, collision will ends when jam ends\n";
                cancelEvent(endRxMsg);
                scheduleAt(endRxTime, endRxMsg);
            }
        }
        else // EtherFrame or EtherPauseFrame
        {
            numConcurrentTransmissions++;
            if (endRxMsg->getArrivalTime() < endRxTime)
            {
                // otherwise just wait until the end of the longest transmission
                EV << "Overlapping receptions -- setting collision state and extending collision period\n";
                cancelEvent(endRxMsg);
                scheduleAt(endRxTime, endRxMsg);
            }
            else
            {
                EV << "Overlapping receptions -- setting collision state\n";
            }
        }

        // delete collided frames: arrived frame as well as the one we're currently receiving
        delete msg;

        if (receiveState == RECEIVING_STATE)
        {
            delete frameBeingReceived;
            frameBeingReceived = NULL;

            numCollisions++;
            emit(collisionSignal, 1L);
        }

        // go to collision state
        receiveState = RX_COLLISION_STATE;
    }
}


void EtherMAC::handleEndIFGPeriod()
{
    if (transmitState != WAIT_IFG_STATE && transmitState != SEND_IFG_STATE)
        error("Not in WAIT_IFG_STATE at the end of IFG period");

    if (NULL == curTxFrame)
        error("End of IFG and no frame to transmit");

    // End of IFG period, okay to transmit, if Rx idle OR duplexMode
    EV << "IFG elapsed, now begin transmission of frame " << curTxFrame << endl;

    // Perform carrier extension if in Gigabit Ethernet
    if (carrierExtension && curTxFrame->getByteLength() < GIGABIT_MIN_FRAME_WITH_EXT)
    {
        EV << "Performing carrier extension of small frame\n";
        curTxFrame->setByteLength(GIGABIT_MIN_FRAME_WITH_EXT);
    }

    // End of IFG period, okay to transmit, if Rx idle OR duplexMode

    // send frame to network
    startFrameTransmission();
}

void EtherMAC::startFrameTransmission()
{
    EV << "Transmitting a copy of frame " << curTxFrame << endl;

    EtherFrame *frame = curTxFrame->dup();

    prepareTxFrame(frame);

    if (ev.isGUI())
        updateConnectionColor(TRANSMITTING_STATE);

    emit(packetSentToLowerSignal, frame);
    send(frame, physOutGate);

    // check for collisions (there might be an ongoing reception which we don't know about, see below)
    if (!duplexMode && receiveState != RX_IDLE_STATE)
    {
        // During the IFG period the hardware cannot listen to the channel,
        // so it might happen that receptions have begun during the IFG,
        // and even collisions might be in progress.
        //
        // But we don't know of any ongoing transmission so we blindly
        // start transmitting, immediately collide and send a jam signal.
        //
        EV << "startFrameTransmission() send JAM signal.\n";
        printState();

        sendJamSignal();
        // numConcurrentTransmissions stays the same: +1 transmission, -1 jam

        if (receiveState == RECEIVING_STATE)
        {
            delete frameBeingReceived;
            frameBeingReceived = NULL;

            numCollisions++;
            emit(collisionSignal, 1L);
        }
        // go to collision state
        receiveState = RX_COLLISION_STATE;
    }
    else
    {
        // no collision
        scheduleEndTxPeriod(frame);

        // only count transmissions in totalSuccessfulRxTxTime if channel is half-duplex
        if (!duplexMode)
            channelBusySince = simTime();
    }
}

void EtherMAC::handleEndTxPeriod()
{
    // we only get here if transmission has finished successfully, without collision
    if (transmitState != TRANSMITTING_STATE || (!duplexMode && receiveState != RX_IDLE_STATE))
        error("End of transmission, and incorrect state detected");

    if (NULL == curTxFrame)
        error("Frame under transmission cannot be found");

    unsigned long curBytes = curTxFrame->getByteLength();
    numFramesSent++;
    numBytesSent += curBytes;
    emit(txPkBytesSignal, curBytes);

    if (dynamic_cast<EtherPauseFrame*>(curTxFrame) != NULL)
    {
        numPauseFramesSent++;
        emit(txPausePkUnitsSignal, ((EtherPauseFrame*)curTxFrame)->getPauseTime());
    }

    EV << "Transmission of " << curTxFrame << " successfully completed\n";
    delete curTxFrame;
    curTxFrame = NULL;
    lastTxFinishTime = simTime();
    getNextFrameFromQueue();

    // only count transmissions in totalSuccessfulRxTxTime if channel is half-duplex
    if (!duplexMode)
    {
        simtime_t dt = simTime() - channelBusySince;
        totalSuccessfulRxTxTime += dt;
    }

    backoffs = 0;

    // check for and obey received PAUSE frames after each transmission
    if (checkAndScheduleEndPausePeriod())
        return;

    beginSendFrames();
}

void EtherMAC::handleEndRxPeriod()
{
    EV << "Frame reception complete\n";
    simtime_t dt = simTime()-channelBusySince;

    if (receiveState == RECEIVING_STATE) // i.e. not RX_COLLISION_STATE
    {
        EtherTraffic *frame = frameBeingReceived;
        frameBeingReceived = NULL;
        frameReceptionComplete(frame);
        totalSuccessfulRxTxTime += dt;
    }
    else
    {
        totalCollisionTime += dt;
    }

    receiveState = RX_IDLE_STATE;
    numConcurrentTransmissions = 0;

    if (transmitState == TX_IDLE_STATE)
    {
        beginSendFrames();
    }
}

void EtherMAC::handleEndBackoffPeriod()
{
    if (transmitState != BACKOFF_STATE)
        error("At end of BACKOFF not in BACKOFF_STATE!");

    if (NULL == curTxFrame)
        error("At end of BACKOFF and not have current tx frame!");

    if (receiveState == RX_IDLE_STATE)
    {
        EV << "Backoff period ended, wait IFG\n";
        scheduleEndIFGPeriod();
    }
    else
    {
        EV << "Backoff period ended but channel not free, idling\n";
        transmitState = TX_IDLE_STATE;
    }
}

void EtherMAC::handleEndJammingPeriod()
{
    if (transmitState != JAMMING_STATE)
        error("At end of JAMMING not in JAMMING_STATE!");

    EV << "Jamming finished, executing backoff\n";
    handleRetransmission();
}

void EtherMAC::sendJamSignal()
{
    cPacket *jam = new EtherJam("JAM_SIGNAL");
    jam->setByteLength(JAM_SIGNAL_BYTES);

    if (ev.isGUI())
        updateConnectionColor(JAMMING_STATE);

    transmissionChannel->forceTransmissionFinishTime(SIMTIME_ZERO);
    emit(packetSentToLowerSignal, jam);
    send(jam, physOutGate);

    scheduleAt(transmissionChannel->getTransmissionFinishTime(), endJammingMsg);
    transmitState = JAMMING_STATE;
}

void EtherMAC::scheduleEndRxPeriod(cPacket *frame)
{
    scheduleAt(simTime() + frame->getDuration(), endRxMsg);
    receiveState = RECEIVING_STATE;
}

void EtherMAC::handleRetransmission()
{
    if (++backoffs > MAX_ATTEMPTS)
    {
        EV << "Number of retransmit attempts of frame exceeds maximum, cancelling transmission of frame\n";
        delete curTxFrame;
        curTxFrame = NULL;
        transmitState = TX_IDLE_STATE;
        backoffs = 0;
        getNextFrameFromQueue();
        // no beginSendFrames(), because end of jam signal sending will trigger it automatically
        return;
    }

    EV << "Executing backoff procedure\n";
    int backoffrange = (backoffs >= BACKOFF_RANGE_LIMIT) ? 1024 : (1 << backoffs);
    int slotNumber = intuniform(0, backoffrange-1);
    simtime_t backofftime = slotNumber * curEtherDescr->slotTime;

    scheduleAt(simTime() + backofftime, endBackoffMsg);
    transmitState = BACKOFF_STATE;

    numBackoffs++;
    emit(backoffSignal, 1L);
}

void EtherMAC::printState()
{
#define CASE(x) case x: EV << #x; break
    EV << "transmitState: ";

    switch (transmitState)
    {
        CASE(TX_IDLE_STATE);
        CASE(WAIT_IFG_STATE);
        CASE(SEND_IFG_STATE);
        CASE(TRANSMITTING_STATE);
        CASE(JAMMING_STATE);
        CASE(BACKOFF_STATE);
        CASE(PAUSE_STATE);
    }

    EV << ",  receiveState: ";
    switch (receiveState)
    {
        CASE(RX_IDLE_STATE);
        CASE(RECEIVING_STATE);
        CASE(RX_COLLISION_STATE);
    }

    EV << ",  backoffs: " << backoffs;
    EV << ",  numConcurrentTransmissions: " << numConcurrentTransmissions;

    if (txQueue.innerQueue)
        EV << ",  queueLength: " << txQueue.innerQueue->queue.length() << endl;
#undef CASE
}

void EtherMAC::finish()
{
    EtherMACBase::finish();

    simtime_t t = simTime();
    simtime_t totalChannelIdleTime = t - totalSuccessfulRxTxTime - totalCollisionTime;
    recordScalar("rx channel idle (%)", 100*(totalChannelIdleTime/t));
    recordScalar("rx channel utilization (%)", 100*(totalSuccessfulRxTxTime/t));
    recordScalar("rx channel collision (%)", 100*(totalCollisionTime/t));
    recordScalar("collisions",     numCollisions);
    recordScalar("backoffs",       numBackoffs);
}

void EtherMAC::refreshConnection(bool connected_par)
{
    Enter_Method_Silent();

    EtherMACBase::refreshConnection(connected_par);

    if (!connected)
    {
        delete frameBeingReceived;
        frameBeingReceived = NULL;
        cancelEvent(endRxMsg);
        cancelEvent(endBackoffMsg);
        cancelEvent(endJammingMsg);
    }
}

void EtherMAC::updateHasSubcribers()
{
    hasSubscribers = false;  // currently we don't fire any notifications
}

void EtherMAC::processMessageWhenNotConnected(cMessage *msg)
{
    EV << "Interface is not connected -- dropping packet " << msg << endl;
    emit(droppedPkBytesIfaceDownSignal, (long)(PK(msg)->getByteLength()));
    numDroppedIfaceDown++;
    delete msg;

    if (txQueue.extQueue)
    {
        if (0 == txQueue.extQueue->getNumPendingRequests())
            txQueue.extQueue->requestPacket();
    }
}

void EtherMAC::processMessageWhenDisabled(cMessage *msg)
{
    EV << "MAC is disabled -- dropping message " << msg << endl;
    delete msg;

    if (txQueue.extQueue)
    {
        if (0 == txQueue.extQueue->getNumPendingRequests())
            txQueue.extQueue->requestPacket();
    }
}

void EtherMAC::handleEndPausePeriod()
{
    if (transmitState != PAUSE_STATE)
        error("At end of PAUSE not in PAUSE_STATE!");

    EV << "Pause finished, resuming transmissions\n";
    beginSendFrames();
}

void EtherMAC::frameReceptionComplete(EtherTraffic *frame)
{
    int pauseUnits;
    EtherPauseFrame *pauseFrame;

    if ((pauseFrame = dynamic_cast<EtherPauseFrame*>(frame)) != NULL)
    {
        pauseUnits = pauseFrame->getPauseTime();
        delete frame;
        numPauseFramesRcvd++;
        emit(rxPausePkUnitsSignal, pauseUnits);
        processPauseCommand(pauseUnits);
    }
    else if ((dynamic_cast<EtherPadding*>(frame)) != NULL)
    {
        delete frame;
    }
    else
    {
        processReceivedDataFrame((EtherFrame *)frame);
    }
}

void EtherMAC::prepareTxFrame(EtherFrame *frame)
{
    if (frame->getSrc().isUnspecified())
        frame->setSrc(address);

    // add preamble and SFD (Starting Frame Delimiter), then send out
    frame->setOrigByteLength(frame->getByteLength());
    frame->addByteLength(PREAMBLE_BYTES+SFD_BYTES);
    bool inBurst = frameBursting && framesSentInBurst;
    int64 minFrameLength =
            inBurst ? curEtherDescr->frameInBurstMinBytes : curEtherDescr->frameMinBytes;

    if (frame->getByteLength() < minFrameLength)
    {
        frame->setByteLength(minFrameLength);
    }
}

void EtherMAC::processReceivedDataFrame(EtherFrame *frame)
{
    emit(packetReceivedFromLowerSignal, frame);

    // bit errors
    if (frame->hasBitError())
    {
        numDroppedBitError++;
        emit(droppedPkBytesBitErrorSignal, (long)(frame->getByteLength()));
        delete frame;
        return;
    }

    // restore original byte length (strip preamble and SFD and external bytes)
    frame->setByteLength(frame->getOrigByteLength());

    // statistics
    unsigned long curBytes = frame->getByteLength();
    numFramesReceivedOK++;
    numBytesReceivedOK += curBytes;
    emit(rxPkBytesOkSignal, curBytes);

    if (!checkDestinationAddress(frame))
        return;

    numFramesPassedToHL++;
    emit(passedUpPkBytesSignal, curBytes);

    emit(packetSentToUpperSignal, frame);
    // pass up to upper layer
    send(frame, "upperLayerOut");
}

void EtherMAC::processPauseCommand(int pauseUnits)
{
    if (transmitState == TX_IDLE_STATE)
    {
        EV << "PAUSE frame received, pausing for " << pauseUnitsRequested << " time units\n";
        if (pauseUnits > 0)
            scheduleEndPausePeriod(pauseUnits);
    }
    else if (transmitState == PAUSE_STATE)
    {
        EV << "PAUSE frame received, pausing for " << pauseUnitsRequested
           << " more time units from now\n";
        cancelEvent(endPauseMsg);

        if (pauseUnits > 0)
            scheduleEndPausePeriod(pauseUnits);
    }
    else
    {
        // transmitter busy -- wait until it finishes with current frame (endTx)
        // and then it'll go to PAUSE state
        EV << "PAUSE frame received, storing pause request\n";
        pauseUnitsRequested = pauseUnits;
    }
}

void EtherMAC::scheduleEndIFGPeriod()
{
    ASSERT(curTxFrame);

    if (frameBursting
            && (simTime() == lastTxFinishTime)
            && (framesSentInBurst < curEtherDescr->maxFramesInBurst)
            && (bytesSentInBurst + (INTERFRAME_GAP_BITS / 8) + curTxFrame->getByteLength()
                    <= curEtherDescr->maxBytesInBurst)
       )
    {
        EtherPadding *gap = new EtherPadding("IFG");
        gap->setBitLength(INTERFRAME_GAP_BITS);
        bytesSentInBurst += gap->getByteLength();
        send(gap, physOutGate);
        transmitState = SEND_IFG_STATE;
        scheduleAt(transmissionChannel->getTransmissionFinishTime(), endIFGMsg);
        // FIXME Check collision?
    }
    else
    {
        EtherPadding gap;
        gap.setBitLength(INTERFRAME_GAP_BITS);
        bytesSentInBurst = 0;
        framesSentInBurst = 0;
        transmitState = WAIT_IFG_STATE;
        scheduleAt(simTime() + transmissionChannel->calculateDuration(&gap), endIFGMsg);
    }
}

void EtherMAC::scheduleEndTxPeriod(cPacket *frame)
{
    // update burst variables
    if (frameBursting)
    {
        bytesSentInBurst += frame->getByteLength();
        framesSentInBurst++;
    }

    scheduleAt(transmissionChannel->getTransmissionFinishTime(), endTxMsg);
    transmitState = TRANSMITTING_STATE;
}

void EtherMAC::scheduleEndPausePeriod(int pauseUnits)
{
    // length is interpreted as 512-bit-time units
    cPacket pause;
    pause.setBitLength(pauseUnits * PAUSE_BITTIME);
    simtime_t pausePeriod = transmissionChannel->calculateDuration(&pause);
    scheduleAt(simTime() + pausePeriod, endPauseMsg);
    transmitState = PAUSE_STATE;
}

bool EtherMAC::checkAndScheduleEndPausePeriod()
{
    if (pauseUnitsRequested > 0)
    {
        // if we received a PAUSE frame recently, go into PAUSE state
        EV << "Going to PAUSE mode for " << pauseUnitsRequested << " time units\n";

        scheduleEndPausePeriod(pauseUnitsRequested);
        pauseUnitsRequested = 0;
        return true;
    }

    return false;
}

void EtherMAC::beginSendFrames()
{
    if (curTxFrame)
    {
        // Other frames are queued, therefore wait IFG period and transmit next frame
        EV << "Transmit next frame in output queue, after IFG period\n";
        scheduleEndIFGPeriod();
    }
    else
    {
        transmitState = TX_IDLE_STATE;
        // No more frames set transmitter to idle
        EV << "No more frames to send, transmitter set to idle\n";
    }
}

