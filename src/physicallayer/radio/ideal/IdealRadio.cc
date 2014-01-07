//
// Copyright (C) 2013 OpenSim Ltd
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

#include "IdealRadio.h"
#include "ModuleAccess.h"
#include "NodeOperations.h"
#include "NodeStatus.h"

Define_Module(IdealRadio);

IdealRadio::IdealRadio()
{
    endTransmissionTimer = NULL;
    transmissionRange = 0;
    bitrate = 0;
    drawCoverage = false;
}

IdealRadio::~IdealRadio()
{
    cancelAndDelete(endTransmissionTimer);
    cancelAndDeleteEndReceptionTimers();
}

void IdealRadio::cancelAndDeleteEndReceptionTimers()
{
    for (EndReceptionTimers::iterator it = endReceptionTimers.begin(); it!=endReceptionTimers.end(); ++it)
    {
        cMessage *endRxTimer = *it;
        cancelAndDelete(endRxTimer);
    }
    endReceptionTimers.clear();
}

void IdealRadio::initialize(int stage)
{
    IdealRadioChannelAccess::initialize(stage);
    EV << "Initializing IdealRadio, stage = " << stage << endl;
    if (stage == INITSTAGE_LOCAL)
    {
        endTransmissionTimer = new cMessage("endTransmission");
        transmissionRange = par("transmissionRange").doubleValue();
        bitrate = par("bitrate").doubleValue();
        drawCoverage = par("drawCoverage");
    }
    else if (stage == INITSTAGE_LAST)
    {
        if (ev.isGUI() && drawCoverage)
            updateDisplayString();
    }
}

void IdealRadio::setRadioMode(RadioMode newRadioMode)
{
    Enter_Method_Silent();
    if (radioMode != newRadioMode)
    {
        cancelAndDeleteEndReceptionTimers();
        EV << "Changing radio mode from " << getRadioModeName(radioMode) << " to " << getRadioModeName(newRadioMode) << ".\n";
        radioMode = newRadioMode;
        emit(radioModeChangedSignal, newRadioMode);
        updateRadioChannelState();
    }
}

void IdealRadio::setRadioChannel(int newRadioChannel)
{
    Enter_Method_Silent();
    if (radioChannel != newRadioChannel)
    {
        EV << "Changing radio channel from " << radioChannel << " to " << newRadioChannel << ".\n";
        radioChannel = newRadioChannel;
        emit(radioChannelChangedSignal, newRadioChannel);
    }
}

void IdealRadio::handleMessageWhenUp(cMessage *message)
{
    if (message->isSelfMessage())
        handleSelfMessage(message);
    else if (message->getArrivalGate() == upperLayerIn)
    {
        if (!message->isPacket())
            handleCommand(message);
        else if (radioMode == RADIO_MODE_TRANSMITTER || radioMode == RADIO_MODE_TRANSCEIVER)
            handleUpperFrame(check_and_cast<cPacket *>(message));
        else
        {
            EV << "Radio is not in transmitter or transceiver mode, dropping frame.\n";
            delete message;
        }
    }
    else if (message->getArrivalGate() == radioIn)
    {
        if (radioMode == RADIO_MODE_RECEIVER || radioMode == RADIO_MODE_TRANSCEIVER)
            handleLowerFrame(check_and_cast<IdealRadioFrame*>(message));
        else
        {
            EV << "Radio is not in receiver or transceiver mode, dropping frame.\n";
            delete message;
        }
    }
    else
    {
        throw cRuntimeError("Unknown arrival gate '%s'.", message->getArrivalGate()->getFullName());
        delete message;
    }
}

IdealRadioFrame *IdealRadio::encapsulatePacket(cPacket *frame)
{
    delete frame->removeControlInfo();
    // Note: we don't set length() of the IdealRadioFrame, because duration will be used everywhere instead
    IdealRadioFrame *radioFrame = new IdealRadioFrame();
    radioFrame->encapsulate(frame);
    radioFrame->setName(frame->getName());
    radioFrame->setTransmissionRange(transmissionRange);
    radioFrame->setDuration(radioFrame->getBitLength() / bitrate);
    radioFrame->setTransmissionStartPosition(getMobility()->getCurrentPosition());
    return radioFrame;
}

void IdealRadio::sendUp(IdealRadioFrame *radioFrame)
{
    cPacket *frame = radioFrame->decapsulate();
    EV << "Sending up " << frame << ".\n";
    delete radioFrame;
    send(frame, upperLayerOut);
}

void IdealRadio::sendDown(IdealRadioFrame *radioFrame)
{
    EV << "Sending down " << radioFrame << ".\n";
    sendToChannel(radioFrame);
    ASSERT(radioFrame->getDuration() != 0);
    scheduleAt(simTime() + radioFrame->getDuration(), endTransmissionTimer);
    updateRadioChannelState();
}

/**
 * If a message is already being transmitted, an error is raised.
 */
void IdealRadio::handleUpperFrame(cPacket *packet)
{
    if (endTransmissionTimer->isScheduled())
        throw cRuntimeError("Received frame from upper layer while already transmitting.");
    IdealRadioFrame *radioFrame = encapsulatePacket(packet);
    EV << "Transmission of " << radioFrame << " started with " << (bitrate/1e6) << "Mbps.\n";
    sendDown(radioFrame);
}

void IdealRadio::handleCommand(cMessage *message)
{
    // TODO:
}

void IdealRadio::handleSelfMessage(cMessage *message)
{
    if (message == endTransmissionTimer) {
        EV << "Transmission successfully completed.\n";
        updateRadioChannelState();
    }
    else
    {
        EV << "Frame is completely received now.\n";
        for (EndReceptionTimers::iterator it = endReceptionTimers.begin(); it != endReceptionTimers.end(); ++it)
        {
            if (*it == message)
            {
                endReceptionTimers.erase(it);
                IdealRadioFrame *radioFrame = check_and_cast<IdealRadioFrame *>(message->removeControlInfo());
                delete message;
                sendUp(radioFrame);
                updateRadioChannelState();
                return;
            }
        }
        throw cRuntimeError("Self message not found in endReceptionTimers.");
    }
}

/**
 * This function is called right after a packet arrived, i.e. right
 * before it is buffered for 'transmission time'.
 *
 * The message is not treated as noise if the transmissionRange of the
 * received signal is higher than the distance.
 */
void IdealRadio::handleLowerFrame(IdealRadioFrame *radioFrame)
{
    EV << "Reception of " << radioFrame << " started.\n";

    cMessage *endReceptionTimer = new cMessage("endReception");
    endReceptionTimer->setControlInfo(radioFrame);
    endReceptionTimers.push_back(endReceptionTimer);

    // NOTE: use arrivalTime instead of simTime, because we might be calling this
    // function during a channel change, when we're picking up ongoing transmissions
    // on the channel -- and then the message's arrival time is in the past!
    scheduleAt(radioFrame->getArrivalTime() + radioFrame->getDuration(), endReceptionTimer);
    updateRadioChannelState();
}

bool IdealRadio::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_PHYSICAL_LAYER)
            setRadioMode(RADIO_MODE_OFF);
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_PHYSICAL_LAYER)
            setRadioMode(RADIO_MODE_OFF);
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_LOCAL)
            setRadioMode(RADIO_MODE_OFF);
    }
    return true;
}

void IdealRadio::updateRadioChannelState()
{
    RadioChannelState newRadioChannelState;
    if (radioMode == RADIO_MODE_OFF || radioMode == RADIO_MODE_SLEEP)
        newRadioChannelState = RADIO_CHANNEL_STATE_UNKNOWN;
    else if (endTransmissionTimer->isScheduled())
        newRadioChannelState = RADIO_CHANNEL_STATE_TRANSMITTING;
    else if (endReceptionTimers.size() > 0)
        newRadioChannelState = RADIO_CHANNEL_STATE_RECEIVING;
    else if (false)
        newRadioChannelState = RADIO_CHANNEL_STATE_BUSY;
    else
        newRadioChannelState = RADIO_CHANNEL_STATE_FREE;
    if (radioChannelState != newRadioChannelState)
    {
        EV << "Changing radio channel state from " << getRadioChannelStateName(radioChannelState) << " to " << getRadioChannelStateName(newRadioChannelState) << ".\n";
        radioChannelState = newRadioChannelState;
        emit(radioChannelStateChangedSignal, newRadioChannelState);
    }
}

void IdealRadio::updateDisplayString()
{
    if (radioChannelEntry) {
        cDisplayString& d = node->getDisplayString();
        // FIXME this overrides the ranges if more than one radio is present is a host
        d.removeTag("r1");
        d.insertTag("r1");
        d.setTagArg("r1", 0, (long)transmissionRange);  // TODO
        d.setTagArg("r1", 2, "gray");
        d.removeTag("r2");
    }
}
