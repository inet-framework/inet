//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/physicallayer/common/packetlevel/Radio.h"
#include "inet/physicallayer/common/packetlevel/RadioMedium.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

namespace physicallayer {

Define_Module(Radio);

simsignal_t Radio::minSNIRSignal = cComponent::registerSignal("minSNIR");
simsignal_t Radio::packetErrorRateSignal = cComponent::registerSignal("packetErrorRate");
simsignal_t Radio::bitErrorRateSignal = cComponent::registerSignal("bitErrorRate");
simsignal_t Radio::symbolErrorRateSignal = cComponent::registerSignal("symbolErrorRate");

Radio::Radio() :
    id(nextId++),
    antenna(nullptr),
    transmitter(nullptr),
    receiver(nullptr),
    medium(nullptr),
    displayCommunicationRange(false),
    displayInterferenceRange(false),
    upperLayerOut(nullptr),
    upperLayerIn(nullptr),
    radioIn(nullptr),
    radioMode(RADIO_MODE_OFF),
    nextRadioMode(RADIO_MODE_OFF),
    previousRadioMode(RADIO_MODE_OFF),
    receptionState(RECEPTION_STATE_UNDEFINED),
    transmissionState(TRANSMISSION_STATE_UNDEFINED),
    endTransmissionTimer(nullptr),
    endReceptionTimer(nullptr),
    endSwitchTimer(nullptr)
{
}

Radio::~Radio()
{
    // NOTE: can't use the medium module here, because it may have been already deleted
    cModule *medium = getSimulation()->getModule(mediumModuleId);
    if (medium != nullptr)
        check_and_cast<IRadioMedium *>(medium)->removeRadio(this);
    cancelAndDelete(endTransmissionTimer);
    cancelAndDelete(endSwitchTimer);
}

void Radio::initialize(int stage)
{
    PhysicalLayerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        endTransmissionTimer = new cMessage("endTransmission");
        antenna = check_and_cast<IAntenna *>(getSubmodule("antenna"));
        transmitter = check_and_cast<ITransmitter *>(getSubmodule("transmitter"));
        receiver = check_and_cast<IReceiver *>(getSubmodule("receiver"));
        medium = getModuleFromPar<IRadioMedium>(par("radioMediumModule"), this);
        mediumModuleId = check_and_cast<cModule *>(medium)->getId();
        upperLayerIn = gate("upperLayerIn");
        upperLayerOut = gate("upperLayerOut");
        radioIn = gate("radioIn");
        radioIn->setDeliverOnReceptionStart(true);
        displayCommunicationRange = par("displayCommunicationRange");
        displayInterferenceRange = par("displayInterferenceRange");
        WATCH(radioMode);
        WATCH(receptionState);
        WATCH(transmissionState);
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        medium->addRadio(this);
        endSwitchTimer = new cMessage("endSwitch");
        parseRadioModeSwitchingTimes();
    }
    else if (stage == INITSTAGE_LAST) {
        updateDisplayString();
        EV_INFO << "Initialized " << getCompleteStringRepresentation() << endl;
    }
}

std::ostream& Radio::printToStream(std::ostream& stream, int level) const
{
    stream << static_cast<const cSimpleModule *>(this);
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", antenna = " << printObjectToString(antenna, level - 1)
               << ", transmitter = " << printObjectToString(transmitter, level - 1)
               << ", receiver = " << printObjectToString(receiver, level - 1);
    return stream;
}

void Radio::setRadioMode(RadioMode newRadioMode)
{
    Enter_Method_Silent();
    if (newRadioMode < RADIO_MODE_OFF || newRadioMode > RADIO_MODE_SWITCHING)
        throw cRuntimeError("Unknown radio mode: %d", newRadioMode);
    else if (newRadioMode == RADIO_MODE_SWITCHING)
        throw cRuntimeError("Cannot switch manually to RADIO_MODE_SWITCHING");
    else if (radioMode == RADIO_MODE_SWITCHING || endSwitchTimer->isScheduled())
        throw cRuntimeError("Cannot switch to a new radio mode while another switch is in progress");
    else if (newRadioMode != radioMode && newRadioMode != nextRadioMode) {
        simtime_t switchingTime = switchingTimes[radioMode][newRadioMode];
        if (switchingTime != 0)
            startRadioModeSwitch(newRadioMode, switchingTime);
        else
            completeRadioModeSwitch(newRadioMode);
    }
}

void Radio::parseRadioModeSwitchingTimes()
{
    const char *times = par("switchingTimes");

    char prefix[3];
    unsigned int count = sscanf(times, "%s", prefix);

    if (count > 2)
        throw cRuntimeError("Metric prefix should be no more than two characters long");

    double metric = 1;

    if (strcmp("s", prefix) == 0)
        metric = 1;
    else if (strcmp("ms", prefix) == 0)
        metric = 0.001;
    else if (strcmp("ns", prefix) == 0)
        metric = 0.000000001;
    else
        throw cRuntimeError("Undefined or missed metric prefix for switchingTimes parameter");

    cStringTokenizer tok(times + count + 1);
    unsigned int idx = 0;
    while (tok.hasMoreTokens()) {
        switchingTimes[idx / RADIO_MODE_SWITCHING][idx % RADIO_MODE_SWITCHING] = atof(tok.nextToken()) * metric;
        idx++;
    }
    if (idx != RADIO_MODE_SWITCHING * RADIO_MODE_SWITCHING)
        throw cRuntimeError("Check your switchingTimes parameter! Some parameters may be missed");
}

void Radio::startRadioModeSwitch(RadioMode newRadioMode, simtime_t switchingTime)
{
    EV_DETAIL << "Starting to change radio mode from " << getRadioModeName(radioMode) << " to " << getRadioModeName(newRadioMode) << ".\n";
    previousRadioMode = radioMode;
    radioMode = RADIO_MODE_SWITCHING;
    nextRadioMode = newRadioMode;
    emit(radioModeChangedSignal, radioMode);
    scheduleAt(simTime() + switchingTime, endSwitchTimer);
}

void Radio::completeRadioModeSwitch(RadioMode newRadioMode)
{
    EV_DETAIL << "Radio mode changed from " << getRadioModeName(previousRadioMode) << " to " << getRadioModeName(newRadioMode) << endl;
    if (newRadioMode != IRadio::RADIO_MODE_RECEIVER && newRadioMode != IRadio::RADIO_MODE_TRANSCEIVER) {
        endReceptionTimer = nullptr;
    }
    else if (newRadioMode != IRadio::RADIO_MODE_TRANSMITTER && newRadioMode != IRadio::RADIO_MODE_TRANSCEIVER) {
        if (endTransmissionTimer->isScheduled()) {
            EV_WARN << "Aborting ongoing transmissions is not supported" << endl;
            cancelEvent(endTransmissionTimer);
        }
    }
    radioMode = previousRadioMode = nextRadioMode = newRadioMode;
    emit(radioModeChangedSignal, newRadioMode);
    updateTransceiverState();
}

const ITransmission *Radio::getTransmissionInProgress() const
{
    if (!endTransmissionTimer->isScheduled())
        return nullptr;
    else
        return static_cast<RadioFrame *>(endTransmissionTimer->getControlInfo())->getTransmission();
}

const ITransmission *Radio::getReceptionInProgress() const
{
    if (!endReceptionTimer)
        return nullptr;
    else
        return static_cast<RadioFrame *>(endReceptionTimer->getControlInfo())->getTransmission();
}

void Radio::handleMessageWhenDown(cMessage *message)
{
    if (message->getArrivalGate() == radioIn || isReceptionEndTimer(message))
        delete message;
    else
        OperationalBase::handleMessageWhenDown(message);
}

void Radio::handleMessageWhenUp(cMessage *message)
{
    if (message->isSelfMessage())
        handleSelfMessage(message);
    else if (message->getArrivalGate() == upperLayerIn) {
        if (!message->isPacket()) {
            handleUpperCommand(message);
            delete message;
        }
        else if (radioMode == RADIO_MODE_TRANSMITTER || radioMode == RADIO_MODE_TRANSCEIVER)
            startTransmission(check_and_cast<cPacket *>(message));
        else {
            EV_ERROR << "Radio is not in transmitter or transceiver mode, dropping frame.\n";
            delete message;
        }
    }
    else if (message->getArrivalGate() == radioIn) {
        if (!message->isPacket()) {
            handleLowerCommand(message);
            delete message;
        }
        else
            startReception(check_and_cast<RadioFrame *>(message));
    }
    else
        throw cRuntimeError("Unknown arrival gate '%s'.", message->getArrivalGate()->getFullName());
}

void Radio::handleSelfMessage(cMessage *message)
{
    if (message == endTransmissionTimer)
        endTransmission();
    else if (message == endSwitchTimer)
        completeRadioModeSwitch(nextRadioMode);
    else if (isReceptionEndTimer(message))
        endReception(message);
    else
        throw cRuntimeError("Unknown self message");
}

void Radio::handleUpperCommand(cMessage *message)
{
    if (message->getKind() == RADIO_C_CONFIGURE) {
        ConfigureRadioCommand *configureCommand = check_and_cast<ConfigureRadioCommand *>(message->getControlInfo());
        if (configureCommand->getRadioMode() != -1)
            setRadioMode((RadioMode)configureCommand->getRadioMode());
    }
    else
        throw cRuntimeError("Unsupported command");
}

void Radio::handleLowerCommand(cMessage *message)
{
    throw cRuntimeError("Unsupported command");
}

bool Radio::handleNodeStart(IDoneCallback *doneCallback)
{
    // NOTE: we ignore switching time during start
    completeRadioModeSwitch(RADIO_MODE_OFF);
    return PhysicalLayerBase::handleNodeStart(doneCallback);
}

bool Radio::handleNodeShutdown(IDoneCallback *doneCallback)
{
    // NOTE: we ignore switching time during shutdown
    completeRadioModeSwitch(RADIO_MODE_OFF);
    return PhysicalLayerBase::handleNodeShutdown(doneCallback);
}

void Radio::handleNodeCrash()
{
    completeRadioModeSwitch(RADIO_MODE_OFF);
}

void Radio::startTransmission(cPacket *macFrame)
{
    if (endTransmissionTimer->isScheduled())
        throw cRuntimeError("Received frame from upper layer while already transmitting.");
    const RadioFrame *radioFrame = check_and_cast<const RadioFrame *>(medium->transmitPacket(this, macFrame));
    EV_INFO << "Transmission of " << (IRadioFrame *)radioFrame << " as " << radioFrame->getTransmission() << " is started.\n";
    ASSERT(radioFrame->getDuration() != 0);
    endTransmissionTimer->setControlInfo(const_cast<RadioFrame *>(radioFrame));
    scheduleAt(simTime() + radioFrame->getDuration(), endTransmissionTimer);
    updateTransceiverState();
    delete macFrame->removeControlInfo();
}

void Radio::endTransmission()
{
    RadioFrame *radioFrame = static_cast<RadioFrame *>(endTransmissionTimer->removeControlInfo());
    EV_INFO << "Transmission of " << (IRadioFrame *)radioFrame << " as " << radioFrame->getTransmission() << " is completed.\n";
    updateTransceiverState();
    delete radioFrame;
}

void Radio::startReception(RadioFrame *radioFrame)
{
    const ITransmission *transmission = radioFrame->getTransmission();
    const IArrival *arrival = medium->getArrival(this, radioFrame->getTransmission());
    cMessage *timer = new cMessage("endReception");
    timer->setControlInfo(radioFrame);
    if (arrival->getStartTime() == simTime()) {
        bool isReceptionAttempted = (radioMode == RADIO_MODE_RECEIVER || radioMode == RADIO_MODE_TRANSCEIVER) && medium->isReceptionAttempted(this, transmission);
        EV_INFO << "Reception of " << (IRadioFrame *)radioFrame << " as " << transmission << " is " << (isReceptionAttempted ? "attempted" : "ignored") << ".\n";
        if (isReceptionAttempted)
            endReceptionTimer = timer;
    }
    scheduleAt(arrival->getEndTime(), timer);
    updateTransceiverState();
}

void Radio::endReception(cMessage *message)
{
    RadioFrame *radioFrame = static_cast<RadioFrame *>(message->getControlInfo());
    EV_INFO << "Reception of " << (IRadioFrame *)radioFrame << " as " << radioFrame->getTransmission() << " is completed.\n";
    if ((radioMode == RADIO_MODE_RECEIVER || radioMode == RADIO_MODE_TRANSCEIVER) && message == endReceptionTimer) {
        cPacket *macFrame = medium->receivePacket(this, radioFrame);
        EV_INFO << "Sending up " << macFrame << ".\n";
        const ReceptionIndication *indication = check_and_cast<const ReceptionIndication *>(macFrame->getControlInfo());
        emit(minSNIRSignal, indication->getMinSNIR());
        if (!isNaN(indication->getPacketErrorRate()))
            emit(packetErrorRateSignal, indication->getPacketErrorRate());
        if (!isNaN(indication->getBitErrorRate()))
            emit(bitErrorRateSignal, indication->getBitErrorRate());
        if (!isNaN(indication->getSymbolErrorRate()))
            emit(symbolErrorRateSignal, indication->getSymbolErrorRate());
        send(macFrame, upperLayerOut);
        endReceptionTimer = nullptr;
    }
    delete message;
    updateTransceiverState();
}

bool Radio::isReceptionEndTimer(cMessage *message)
{
    return !strcmp(message->getName(), "endReception");
}

bool Radio::isListeningPossible()
{
    const simtime_t now = simTime();
    const Coord position = antenna->getMobility()->getCurrentPosition();
    // TODO: use 2 * minInterferenceTime for lookahead? or maybe simply use 0 duration listening?
    const IListening *listening = receiver->createListening(this, now, now + 1E-12, position, position);
    const IListeningDecision *listeningDecision = medium->listenOnMedium(this, listening);
    bool isListeningPossible = listeningDecision->isListeningPossible();
    delete listening;
    delete listeningDecision;
    return isListeningPossible;
}

void Radio::updateTransceiverState()
{
    // reception state
    ReceptionState newRadioReceptionState;
    if (radioMode == RADIO_MODE_OFF || radioMode == RADIO_MODE_SLEEP || radioMode == RADIO_MODE_TRANSMITTER)
        newRadioReceptionState = RECEPTION_STATE_UNDEFINED;
    else if (endReceptionTimer && endReceptionTimer->isScheduled())
        newRadioReceptionState = RECEPTION_STATE_RECEIVING;
    else if (false) // TODO: synchronization model
        newRadioReceptionState = RECEPTION_STATE_SYNCHRONIZING;
    else if (isListeningPossible())
        newRadioReceptionState = RECEPTION_STATE_BUSY;
    else
        newRadioReceptionState = RECEPTION_STATE_IDLE;
    if (receptionState != newRadioReceptionState) {
        EV_INFO << "Changing radio reception state from " << getRadioReceptionStateName(receptionState) << " to " << getRadioReceptionStateName(newRadioReceptionState) << ".\n";
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
    if (transmissionState != newRadioTransmissionState) {
        EV_INFO << "Changing radio transmission state from " << getRadioTransmissionStateName(transmissionState) << " to " << getRadioTransmissionStateName(newRadioTransmissionState) << ".\n";
        transmissionState = newRadioTransmissionState;
        emit(transmissionStateChangedSignal, newRadioTransmissionState);
    }
}

void Radio::updateDisplayString()
{
    // draw the interference area and sensitivity area
    // according pathloss propagation only
    // we use the radio channel method to calculate interference distance
    // it should be the methods provided by propagation models, but to
    // avoid a big modification, we reuse those methods.
    if (hasGUI() && (displayInterferenceRange || displayCommunicationRange)) {
        cModule *host = findContainingNode(this);
        cDisplayString& displayString = host->getDisplayString();
        if (displayInterferenceRange) {
            m maxInterferenceRage = check_and_cast<const RadioMedium *>(medium)->getMediumLimitCache()->getMaxInterferenceRange(this);
            char tag[32];
            sprintf(tag, "r%i1", getId());
            displayString.removeTag(tag);
            displayString.insertTag(tag);
            displayString.setTagArg(tag, 0, maxInterferenceRage.get());
            displayString.setTagArg(tag, 2, "gray");
        }
        if (displayCommunicationRange) {
            m maxCommunicationRange = check_and_cast<const RadioMedium *>(medium)->getMediumLimitCache()->getMaxCommunicationRange(this);
            char tag[32];
            sprintf(tag, "r%i2", getId());
            displayString.removeTag(tag);
            displayString.insertTag(tag);
            displayString.setTagArg(tag, 0, maxCommunicationRange.get());
            displayString.setTagArg(tag, 2, "blue");
        }
    }
}

} // namespace physicallayer

} // namespace inet

