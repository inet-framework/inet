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

#include "NewRadio.h"
#include "NewRadioChannel.h"
#include "ModuleAccess.h"
#include "NodeOperations.h"
#include "NodeStatus.h"
// TODO: should not be here
#include "ScalarImplementation.h"
// TODO: should not be here
#include "PhyControlInfo_m.h"
// TODO: should not be here
#include "Ieee80211Consts.h"

Define_Module(NewRadio);

NewRadio::NewRadio()
{
    endTransmissionTimer = NULL;
    endReceptionTimer = NULL;
}

NewRadio::~NewRadio()
{
    cancelAndDelete(endTransmissionTimer);
    cancelAndDeleteEndReceptionTimers();
}

void NewRadio::initialize(int stage)
{
    Radio::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        endTransmissionTimer = new cMessage("endTransmission");
        antenna = check_and_cast<IRadioAntenna *>(getSubmodule("antenna"));
        transmitter = check_and_cast<IRadioSignalTransmitter *>(getSubmodule("transmitter"));
        receiver = check_and_cast<IRadioSignalReceiver *>(getSubmodule("receiver"));
        channel = check_and_cast<IRadioChannel *>(simulation.getModuleByPath("radioChannel"));
        channel->addRadio(this);
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER)
    {
        // TODO: KLUDGE: push down to ScalarRadio and Ieee80211Radio?
        if (hasPar("channelNumber")) {
            int newChannelNumber = par("channelNumber");
            Hz carrierFrequency = Hz(CENTER_FREQUENCIES[newChannelNumber + 1]);
            ScalarRadioSignalTransmitter *scalarTransmitter = const_cast<ScalarRadioSignalTransmitter *>(check_and_cast<const ScalarRadioSignalTransmitter *>(transmitter));
            ScalarRadioSignalReceiver *scalarReceiver = const_cast<ScalarRadioSignalReceiver *>(check_and_cast<const ScalarRadioSignalReceiver *>(receiver));
            scalarTransmitter->setCarrierFrequency(carrierFrequency);
            scalarReceiver->setCarrierFrequency(carrierFrequency);
            setOldRadioChannel(newChannelNumber);
        }
    }
    else if (stage == INITSTAGE_LAST)
    {
        EV_DEBUG << "Radio initialized with " << antenna << ", " << transmitter << ", " << receiver << endl;
    }
}

void NewRadio::setRadioMode(RadioMode newRadioMode)
{
    Enter_Method_Silent();
    if (radioMode != newRadioMode)
    {
        if (newRadioMode == OldIRadio::RADIO_MODE_RECEIVER)
        {
            // KLUDGE: to keep fingerprint
            for (EndReceptionTimers::iterator it = endReceptionTimers.begin(); it != endReceptionTimers.end(); it++)
            {
                cMessage *timer = *it;
                RadioFrame *radioFrame = static_cast<RadioFrame*>(timer->getContextPointer());
                const IRadioSignalTransmission *transmission = radioFrame->getTransmission();
                const IRadioSignalArrival *arrival = channel->getPropagation()->computeArrival(transmission, antenna->getMobility());
                channel->setArrival(this, transmission, arrival);
                simtime_t startArrivalTime = arrival->getStartTime();
                simtime_t endArrivalTime = arrival->getEndTime();
//                double distance = antenna->getMobility()->getCurrentPosition().distance(transmission->getStartPosition());
//                simtime_t propagationTime = distance / SPEED_OF_LIGHT;
//                simtime_t startArrivalTime = transmission->getStartTime() + propagationTime;
//                simtime_t endArrivalTime = startArrivalTime + radioFrame->getDuration();
                EV << "Picking up " << (IRadioFrame *)radioFrame << endl;
                if (startArrivalTime >= simTime())
                    sendDirect(radioFrame, startArrivalTime - simTime(), radioFrame->getDuration(), getContainingNode(this), getRadioGate()->getId());
                else if (endArrivalTime > simTime())
                    scheduleAt(endArrivalTime, timer);
            }
        }
        else
        {
            // KLUDGE: to keep fingerprint
            endReceptionTimer = NULL;
            for (EndReceptionTimers::iterator it = endReceptionTimers.begin(); it != endReceptionTimers.end(); it++)
            {
                cMessage *timer = *it;
                timer->setKind(false);
                cancelEvent(timer);
            }
        }
// TODO: was cancelAndDeleteEndReceptionTimers();
        EV << "Changing radio mode from " << getRadioModeName(radioMode) << " to " << getRadioModeName(newRadioMode) << ".\n";
        radioMode = newRadioMode;
        emit(radioModeChangedSignal, newRadioMode);
        updateTransceiverState();
    }
}

// KLUDGE: to keep fingerprint
void NewRadio::setOldRadioChannel(int newRadioChannel)
{
    Radio::setOldRadioChannel(newRadioChannel);
    // TODO: what about those receptions that are ongoing on the new channel?
    cancelAndDeleteEndReceptionTimers();
}

const IRadioSignalTransmission *NewRadio::getTransmissionInProgress() const
{
    if (endTransmissionTimer)
        return static_cast<RadioFrame*>(endTransmissionTimer->getContextPointer())->getTransmission();
    else
        return NULL;
}

const IRadioSignalTransmission *NewRadio::getReceptionInProgress() const
{
    if (endReceptionTimer)
        return static_cast<RadioFrame*>(endReceptionTimer->getContextPointer())->getTransmission();
    else
        return NULL;
}

void NewRadio::handleMessageWhenUp(cMessage *message)
{
    if (message->isSelfMessage())
        handleSelfMessage(message);
    else if (message->getArrivalGate() == upperLayerIn)
    {
        if (!message->isPacket())
            handleUpperCommand(message);
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
        if (!message->isPacket())
            handleLowerCommand(message);
        else if (radioMode == RADIO_MODE_RECEIVER || radioMode == RADIO_MODE_TRANSCEIVER)
            handleLowerFrame(check_and_cast<RadioFrame*>(message));
        else
        {
            // KLUDGE: fingerprint
            {
                cMessage *timer = new cMessage("endReception");
                timer->setKind(false);
                timer->setContextPointer(message);
                endReceptionTimers.push_back(timer);
            }
            EV << "Radio is not in receiver or transceiver mode, dropping frame.\n";
//            delete message;
        }
    }
    else
    {
        throw cRuntimeError("Unknown arrival gate '%s'.", message->getArrivalGate()->getFullName());
        delete message;
    }
}

void NewRadio::handleSelfMessage(cMessage *message)
{
    if (message == endTransmissionTimer) {
        EV << "Transmission successfully completed.\n";
        updateTransceiverState();
    }
    else
    {
        EV << "Frame is completely received now.\n";
        for (EndReceptionTimers::iterator it = endReceptionTimers.begin(); it != endReceptionTimers.end(); it++)
        {
            if (*it == message)
            {
                endReceptionTimers.erase(it);
                RadioFrame *radioFrame = static_cast<RadioFrame *>(message->getContextPointer());
                if (message->getKind())
                {
                    cPacket *macFrame = receivePacket(radioFrame);
                    EV << "Sending up " << macFrame << ".\n";
                    send(macFrame, upperLayerOut);
                    endReceptionTimer = NULL;
                }
                updateTransceiverState();
                delete radioFrame;
                delete message;
                return;
            }
        }
        throw cRuntimeError("Self message not found in endReceptionTimers.");
    }
}

void NewRadio::handleUpperCommand(cMessage *message)
{
    // TODO: revise interface
    if (message->getKind() == PHY_C_CONFIGURERADIO)
    {
        PhyControlInfo *phyControlInfo = check_and_cast<PhyControlInfo *>(message->getControlInfo());
        int newChannelNumber = phyControlInfo->getChannelNumber();
        bps newBitrate = bps(phyControlInfo->getBitrate());
        delete phyControlInfo;

        // TODO: KLUDGE: push down to ScalarRadio and Ieee80211Radio?
        ScalarRadioSignalTransmitter *scalarTransmitter = const_cast<ScalarRadioSignalTransmitter *>(check_and_cast<const ScalarRadioSignalTransmitter *>(transmitter));
        ScalarRadioSignalReceiver *scalarReceiver = const_cast<ScalarRadioSignalReceiver *>(check_and_cast<const ScalarRadioSignalReceiver *>(receiver));
        if (newChannelNumber != -1)
        {
            Hz carrierFrequency = Hz(CENTER_FREQUENCIES[newChannelNumber + 1]);
            scalarTransmitter->setCarrierFrequency(carrierFrequency);
            scalarReceiver->setCarrierFrequency(carrierFrequency);
            // KLUDGE: channel
            setOldRadioChannel(newChannelNumber);
        }
        else if (newBitrate.get() != -1)
        {
            scalarTransmitter->setBitrate(newBitrate);
        }
    }
    else
        throw cRuntimeError("Unsupported command");
}

void NewRadio::handleLowerCommand(cMessage *message)
{
    throw cRuntimeError("Unsupported command");
}

void NewRadio::handleUpperFrame(cPacket *packet)
{
    if (endTransmissionTimer->isScheduled())
        throw cRuntimeError("Received frame from upper layer while already transmitting.");
    const RadioFrame *radioFrame = check_and_cast<const RadioFrame *>(transmitPacket(packet, simTime()));
    endTransmissionTimer->setContextPointer(const_cast<RadioFrame *>(radioFrame));
    channel->sendToChannel(this, radioFrame);
    EV << "Transmission of " << (IRadioFrame *)radioFrame << " as " << radioFrame->getTransmission() << " is started.\n";
    ASSERT(radioFrame->getDuration() != 0);
    scheduleAt(simTime() + radioFrame->getDuration(), endTransmissionTimer);
    updateTransceiverState();
    delete radioFrame;
}

void NewRadio::handleLowerFrame(RadioFrame *radioFrame)
{
    const IRadioSignalTransmission *transmission = radioFrame->getTransmission();
    bool isReceptionAttempted = channel->isReceptionAttempted(this, transmission);
    EV << "Reception of " << (IRadioFrame *)radioFrame << " as " << transmission << " is " << (isReceptionAttempted ? "attempted" : "ignored") << ".\n";
    cMessage *timer = new cMessage("endReception");
    timer->setContextPointer(radioFrame);
    timer->setKind(isReceptionAttempted);
    if (isReceptionAttempted)
    {
        //ASSERT(!endReceptionTimer);
        endReceptionTimer = timer;
    }
    endReceptionTimers.push_back(timer);
// TODO:   scheduleAt(receptionDecision->getReception()->getEndTime(), newEndReceptionTimer);
    scheduleAt(simTime() + radioFrame->getDuration(), timer);
    updateTransceiverState();
}

bool NewRadio::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
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

void NewRadio::cancelAndDeleteEndReceptionTimers()
{
    for (EndReceptionTimers::iterator it = endReceptionTimers.begin(); it != endReceptionTimers.end(); it++) {
        cMessage *timer = *it;
        RadioFrame *radioFrame = static_cast<RadioFrame*>(timer->getContextPointer());
        delete radioFrame;
        cancelAndDelete(timer);
    }
    endReceptionTimers.clear();
}

void NewRadio::updateTransceiverState()
{
    // reception state
    ReceptionState newRadioReceptionState;
    const simtime_t now = simTime();
    const Coord position = antenna->getMobility()->getCurrentPosition();
    // TODO: use 2 * minInterferenceTime for lookahead?
    // TODO: maybe simply use 0 duration listening?
    const IRadioSignalListeningDecision *listeningDecision = channel->listenOnChannel(this, receiver->createListening(this, now, now + 1E-12, position, position));
    if (radioMode == RADIO_MODE_OFF || radioMode == RADIO_MODE_SLEEP || radioMode == RADIO_MODE_TRANSMITTER)
        newRadioReceptionState = RECEPTION_STATE_UNDEFINED;
    else if (endReceptionTimer && endReceptionTimer->isScheduled())
        newRadioReceptionState = RECEPTION_STATE_RECEIVING;
    else if (false) // NOTE: synchronization is not modeled in New radio
        newRadioReceptionState = RECEPTION_STATE_SYNCHRONIZING;
    else if (listeningDecision->isListeningPossible())
        newRadioReceptionState = RECEPTION_STATE_BUSY;
    else
        newRadioReceptionState = RECEPTION_STATE_IDLE;
    if (receptionState != newRadioReceptionState)
    {
        EV << "Changing radio reception state from " << getRadioReceptionStateName(receptionState) << " to " << getRadioReceptionStateName(newRadioReceptionState) << ".\n";
        receptionState = newRadioReceptionState;
        emit(receptionStateChangedSignal, newRadioReceptionState);
    }
    // transmission state
    TransmissionState newRadioTransmissionState;
    if (radioMode == RADIO_MODE_OFF || radioMode == RADIO_MODE_SLEEP || radioMode == RADIO_MODE_RECEIVER)
        newRadioTransmissionState = TRANSMISSION_STATE_UNDEFINED;
    else if (endTransmissionTimer->isScheduled())
        newRadioTransmissionState = TRANSMISSION_STATE_TRANSMITTING;
    else
        newRadioTransmissionState = TRANSMISSION_STATE_IDLE;
    if (transmissionState != newRadioTransmissionState)
    {
        EV << "Changing radio transmission state from " << getRadioTransmissionStateName(transmissionState) << " to " << getRadioTransmissionStateName(newRadioTransmissionState) << ".\n";
        transmissionState = newRadioTransmissionState;
        emit(transmissionStateChangedSignal, newRadioTransmissionState);
    }
}
