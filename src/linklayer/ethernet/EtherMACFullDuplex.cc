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

#include "EtherMACFullDuplex.h"

#include "EtherFrame_m.h"
#include "IPassiveQueue.h"
#include "NotificationBoard.h"
#include "NotifierConsts.h"

// TODO: refactor using a statemachine that is present in a single function
// TODO: this helps understanding what interactions are there and how they affect the state

Define_Module(EtherMACFullDuplex);

EtherMACFullDuplex::EtherMACFullDuplex()
{
}

void EtherMACFullDuplex::initialize()
{
    EtherMACBase::initialize();
    if (!par("duplexMode").boolValue())
        throw cRuntimeError("Half duplex operation is not supported by EtherMACFullDuplex, use the EtherMAC module for that! (Please enable csmacdSupport on EthernetInterface)");

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
        readChannelParameters(true);

    if (msg->isSelfMessage())
        handleSelfMessage(msg);
    else if (msg->getArrivalGate() == upperLayerInGate)
        processFrameFromUpperLayer(check_and_cast<EtherFrame *>(msg));
    else if (msg->getArrivalGate() == physInGate)
        processMsgFromNetwork(check_and_cast<EtherTraffic *>(msg));
    else
        throw cRuntimeError("Message received from unknown gate!");

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
        throw cRuntimeError("Unknown self message received!");
}

void EtherMACFullDuplex::startFrameTransmission()
{
    EV << "Transmitting a copy of frame " << curTxFrame << endl;

    EtherFrame *frame = curTxFrame->dup();  // note: we need to duplicate the frame because we emit a signal with it in endTxPeriod()

    if (frame->getSrc().isUnspecified())
        frame->setSrc(address);

    if (frame->getByteLength() < curEtherDescr->frameMinBytes)
        frame->setByteLength(curEtherDescr->frameMinBytes);

    // add preamble and SFD (Starting Frame Delimiter), then send out
    frame->addByteLength(PREAMBLE_BYTES+SFD_BYTES);

    // send
    EV << "Starting transmission of " << frame << endl;
    send(frame, physOutGate);

    scheduleAt(transmissionChannel->getTransmissionFinishTime(), endTxMsg);
    transmitState = TRANSMITTING_STATE;
}

void EtherMACFullDuplex::processFrameFromUpperLayer(EtherFrame *frame)
{
    if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
        throw cRuntimeError("Ethernet frame too short, must be at least 64 bytes (padding should be done at encapsulation)");

    frame->setFrameByteLength(frame->getByteLength());

    EV << "Received frame from upper layer: " << frame << endl;

    emit(packetReceivedFromUpperSignal, frame);

    if (frame->getDest().equals(address))
    {
        error("logic error: frame %s from higher layer has local MAC address as dest (%s)",
                frame->getFullName(), frame->getDest().str().c_str());
    }

    if (frame->getByteLength() > MAX_ETHERNET_FRAME_BYTES)
    {
        error("packet from higher layer (%d bytes) exceeds maximum Ethernet frame size (%d)",
                (int)(frame->getByteLength()), MAX_ETHERNET_FRAME_BYTES);
    }

    if (!connected || disabled)
    {
        EV << (!connected ? "Interface is not connected" : "MAC is disabled") << " -- dropping packet " << frame << endl;
        emit(dropPkFromHLIfaceDownSignal, frame);
        numDroppedPkFromHLIfaceDown++;
        delete frame;

        requestNextFrameFromExtQueue();
        return;
    }

    // fill in src address if not set
    if (frame->getSrc().isUnspecified())
        frame->setSrc(address);

    bool isPauseFrame = (dynamic_cast<EtherPauseFrame*>(frame) != NULL);

    if (!isPauseFrame)
    {
        numFramesFromHL++;
        emit(rxPkFromHLSignal, frame);
    }

    if (txQueue.extQueue)
    {
        ASSERT(curTxFrame == NULL);
        curTxFrame = frame;
    }
    else
    {
        if (txQueue.innerQueue->isFull())
            error("txQueue length exceeds %d -- this is probably due to "
                  "a bogus app model generating excessive traffic "
                  "(or if this is normal, increase txQueueLimit!)",
                  txQueue.innerQueue->getQueueLimit());
        // store frame and possibly begin transmitting
        EV << "Frame " << frame << " arrived from higher layers, enqueueing\n";
        txQueue.innerQueue->insertFrame(frame);

        if (!curTxFrame && !txQueue.innerQueue->empty())
            curTxFrame = (EtherFrame*)txQueue.innerQueue->pop();
    }

    if (transmitState == TX_IDLE_STATE)
        scheduleEndIFGPeriod();
}

void EtherMACFullDuplex::processMsgFromNetwork(EtherTraffic *msg)
{
    EV << "Received frame from network: " << msg << endl;

    if (!connected || disabled)
    {
        EV << (!connected ? "Interface is not connected" : "MAC is disabled") << " -- dropping msg " << msg << endl;
        if (dynamic_cast<EtherFrame *>(msg))    // do not count JAM and IFG packets
        {
            emit(dropPkIfaceDownSignal, msg);
            numDroppedIfaceDown++;
        }
        delete msg;

        return;
    }

    EtherFrame *frame = dynamic_cast<EtherFrame *>(msg);
    if (!frame)
    {
        if (dynamic_cast<EtherIFG *>(msg))
            throw cRuntimeError("There is no burst mode in full-duplex operation: EtherIFG is unexpected");
        check_and_cast<EtherFrame *>(msg);
    }

    totalSuccessfulRxTime += frame->getDuration();

    // bit errors
    if (frame->hasBitError())
    {
        numDroppedBitError++;
        emit(dropPkBitErrorSignal, frame);
        delete frame;
        return;
    }

    if (!dropFrameNotForUs(frame))
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
    if (transmitState != WAIT_IFG_STATE)
        error("Not in WAIT_IFG_STATE at the end of IFG period");

    if (NULL == curTxFrame)
        error("End of IFG and no frame to transmit");

    // End of IFG period, okay to transmit
    EV << "IFG elapsed, now begin transmission of frame " << curTxFrame << endl;

    startFrameTransmission();
}

void EtherMACFullDuplex::handleEndTxPeriod()
{
    // we only get here if transmission has finished successfully
    if (transmitState != TRANSMITTING_STATE)
        error("End of transmission, and incorrect state detected");

    if (NULL == curTxFrame)
        error("Frame under transmission cannot be found");

    emit(packetSentToLowerSignal, curTxFrame);  //consider: emit with start time of frame

    if (dynamic_cast<EtherPauseFrame*>(curTxFrame) != NULL)
    {
        numPauseFramesSent++;
        emit(txPausePkUnitsSignal, ((EtherPauseFrame*)curTxFrame)->getPauseTime());
    }
    else
    {
        unsigned long curBytes = curTxFrame->getFrameByteLength();
        numFramesSent++;
        numBytesSent += curBytes;
        emit(txPkSignal, curTxFrame);
    }

    EV << "Transmission of " << curTxFrame << " successfully completed\n";
    delete curTxFrame;
    curTxFrame = NULL;
    lastTxFinishTime = simTime();

    if (pauseUnitsRequested > 0)
    {
        // if we received a PAUSE frame recently, go into PAUSE state
        EV << "Going to PAUSE mode for " << pauseUnitsRequested << " time units\n";

        scheduleEndPausePeriod(pauseUnitsRequested);
        pauseUnitsRequested = 0;
    }
    else
    {
        getNextFrameFromQueue();
        beginSendFrames();
    }
}

void EtherMACFullDuplex::finish()
{
    EtherMACBase::finish();

    simtime_t t = simTime();
    simtime_t totalRxChannelIdleTime = t - totalSuccessfulRxTime;
    recordScalar("rx channel idle (%)", 100 * (totalRxChannelIdleTime / t));
    recordScalar("rx channel utilization (%)", 100 * (totalSuccessfulRxTime / t));
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

    // strip physical layer overhead (preamble, SFD) from frame
    frame->setByteLength(frame->getFrameByteLength());

    // statistics
    unsigned long curBytes = frame->getByteLength();
    numFramesReceivedOK++;
    numBytesReceivedOK += curBytes;
    emit(rxPkOkSignal, frame);

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

    EtherIFG gap;
    transmitState = WAIT_IFG_STATE;
    scheduleAt(simTime() + transmissionChannel->calculateDuration(&gap), endIFGMsg);
}

void EtherMACFullDuplex::scheduleEndPausePeriod(int pauseUnits)
{
    // length is interpreted as 512-bit-time units
    cPacket pause;
    pause.setBitLength(pauseUnits * PAUSE_UNIT_BITS);
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

