//
// Copyright (C) 2006 Levente Meszaros
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

#include "EtherMACFullDuplex.h"

#include "EtherFrame_m.h"
#include "IPassiveQueue.h"
#include "NotificationBoard.h"
#include "NotifierConsts.h"


Define_Module(EtherMACFullDuplex);

EtherMACFullDuplex::EtherMACFullDuplex()
{
}

void EtherMACFullDuplex::initialize()
{
    EtherMACBase::initialize();

    beginSendFrames();
}

void EtherMACFullDuplex::initializeStatistics()
{
    EtherMACBase::initializeStatistics();

    // initialize statistics
    totalSuccessfulRxTime = 0.0;
}

void EtherMACFullDuplex::initializeFlags()
{
    EtherMACBase::initializeFlags();

    duplexMode = true;
    physInGate->setDeliverOnReceptionStart(false);
}

void EtherMACFullDuplex::handleMessage(cMessage *msg)
{
    if (dataratesDiffer)
        calculateParameters(true);

    if (msg->isSelfMessage())
        handleSelfMessage(msg);
    else if (!connected)
        processMessageWhenNotConnected(msg);
    else if (disabled)
        processMessageWhenDisabled(msg);
    else if (msg->getArrivalGate() == gate("upperLayerIn"))
        processFrameFromUpperLayer(check_and_cast<EtherFrame *>(msg));
    else if (msg->getArrivalGate() == gate("phys$i"))
        processMsgFromNetwork(check_and_cast<EtherTraffic *>(msg));
    else
        throw cRuntimeError(this, "Message received from unknown gate!");

    if (ev.isGUI())
        updateDisplayString();
}

void EtherMACFullDuplex::handleSelfMessage(cMessage *msg)
{
    EV << "Self-message " << msg << " received\n";

    if (msg == endTxMsg)
        handleEndTxPeriod();
    else if (msg == endIFGMsg)
        handleEndIFGPeriod();
    else if (msg == endPauseMsg)
        handleEndPausePeriod();
    else
        throw cRuntimeError(this, "Unknown self message received!");
}

void EtherMACFullDuplex::startFrameTransmission()
{
    EV << "Transmitting a copy of frame " << curTxFrame << endl;

    EtherFrame *frame = curTxFrame->dup();

    if (frame->getSrc().isUnspecified())
        frame->setSrc(address);

    frame->setOrigByteLength(frame->getByteLength());

    if (frame->getByteLength() < curEtherDescr.frameMinBytes)
        frame->setByteLength(curEtherDescr.frameMinBytes);

    // add preamble and SFD (Starting Frame Delimiter), then send out
    frame->addByteLength(PREAMBLE_BYTES+SFD_BYTES);

    if (hasSubscribers)
    {
        // fire notification
        notifDetails.setPacket(frame);
        nb->fireChangeNotification(NF_PP_TX_BEGIN, &notifDetails);
    }

    // send
    EV << "Starting transmission of " << frame << endl;
    emit(packetSentToLowerSignal, frame);
    send(frame, physOutGate);

    scheduleAt(transmissionChannel->getTransmissionFinishTime(), endTxMsg);
    transmitState = TRANSMITTING_STATE;
}

void EtherMACFullDuplex::processFrameFromUpperLayer(EtherFrame *frame)
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

        if (!curTxFrame && !txQueue.innerQueue->queue.empty())
            curTxFrame = (EtherFrame*)txQueue.innerQueue->queue.pop();
    }

    if (transmitState == TX_IDLE_STATE)
        scheduleEndIFGPeriod();
}

void EtherMACFullDuplex::processMsgFromNetwork(EtherTraffic *msg)
{
    EV << "Received frame from network: " << msg << endl;

    if (dynamic_cast<EtherPadding *>(msg))
    {
        delete msg;
        return;
    }

    EtherFrame *frame = check_and_cast<EtherFrame *>(msg);

    totalSuccessfulRxTime += frame->getDuration();

    if (hasSubscribers)
    {
        // fire notification
        notifDetails.setPacket(frame);
        nb->fireChangeNotification(NF_PP_RX_END, &notifDetails);
    }

    if (checkDestinationAddress(frame))
    {
        if (dynamic_cast<EtherPauseFrame*>(frame) != NULL)
        {
            int pauseUnits = ((EtherPauseFrame*)frame)->getPauseTime();
            delete frame;
            numPauseFramesRcvd++;
            emit(rxPausePkUnitsSignal, pauseUnits);
            processPauseCommand(pauseUnits);
        }
        else
        {
            processReceivedDataFrame((EtherFrame *)frame);
        }
    }
}

void EtherMACFullDuplex::handleEndIFGPeriod()
{
    if (transmitState != WAIT_IFG_STATE && transmitState != SEND_IFG_STATE)
        error("Not in WAIT_IFG_STATE at the end of IFG period");

    if (NULL == curTxFrame)
        error("End of IFG and no frame to transmit");

    // End of IFG period, okay to transmit
    EV << "IFG elapsed, now begin transmission of frame " << curTxFrame << endl;

    startFrameTransmission();
}

void EtherMACFullDuplex::handleEndTxPeriod()
{
    if (hasSubscribers)
    {
        // fire notification
        notifDetails.setPacket(curTxFrame);
        nb->fireChangeNotification(NF_PP_TX_END, &notifDetails);
    }

    if (pauseUnitsRequested > 0)
    {
        // if we received a PAUSE frame recently, go into PAUSE state
        EV << "Going to PAUSE mode for " << pauseUnitsRequested << " time units\n";

        scheduleEndPausePeriod(pauseUnitsRequested);
        pauseUnitsRequested = 0;
        return;
    }

    // we only get here if transmission has finished successfully, without collision
    if (transmitState != TRANSMITTING_STATE)
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

    beginSendFrames();
}

void EtherMACFullDuplex::finish()
{
    EtherMACBase::finish();

    simtime_t t = simTime();
    simtime_t totalChannelIdleTime = t - totalSuccessfulRxTime;
    recordScalar("rx channel idle (%)", 100 * (totalChannelIdleTime / t));
    recordScalar("rx channel utilization (%)", 100 * (totalSuccessfulRxTime / t));
}

void EtherMACFullDuplex::updateHasSubcribers()
{
    hasSubscribers = nb->hasSubscribers(NF_PP_TX_BEGIN) ||
                     nb->hasSubscribers(NF_PP_TX_END) ||
                     nb->hasSubscribers(NF_PP_RX_END);
}

void EtherMACFullDuplex::processMessageWhenNotConnected(cMessage *msg)
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

void EtherMACFullDuplex::processMessageWhenDisabled(cMessage *msg)
{
    EV << "MAC is disabled -- dropping message " << msg << endl;
    delete msg;

    if (txQueue.extQueue)
    {
        if (0 == txQueue.extQueue->getNumPendingRequests())
            txQueue.extQueue->requestPacket();
    }
}

void EtherMACFullDuplex::handleEndPausePeriod()
{
    if (transmitState != PAUSE_STATE)
        error("End of PAUSE event occurred when not in PAUSE_STATE!");

    EV << "Pause finished, resuming transmissions\n";
    beginSendFrames();
}

void EtherMACFullDuplex::processReceivedDataFrame(EtherFrame *frame)
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
    emit(packetSentToUpperSignal, frame);
    // pass up to upper layer
    send(frame, "upperLayerOut");
}

void EtherMACFullDuplex::processPauseCommand(int pauseUnits)
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

void EtherMACFullDuplex::scheduleEndIFGPeriod()
{
    ASSERT(curTxFrame);

    EtherPadding gap;
    gap.setBitLength(INTERFRAME_GAP_BITS);
    transmitState = WAIT_IFG_STATE;
    scheduleAt(simTime() + transmissionChannel->calculateDuration(&gap), endIFGMsg);
}

void EtherMACFullDuplex::scheduleEndPausePeriod(int pauseUnits)
{
    // length is interpreted as 512-bit-time units
    cPacket pause;
    pause.setBitLength(pauseUnits * PAUSE_BITTIME);
    simtime_t pausePeriod = transmissionChannel->calculateDuration(&pause);
    scheduleAt(simTime() + pausePeriod, endPauseMsg);
    transmitState = PAUSE_STATE;
}

void EtherMACFullDuplex::beginSendFrames()
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

