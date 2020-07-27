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

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/physicallayer/common/packetlevel/Radio.h"
#include "inet/physicallayer/common/packetlevel/RadioMedium.h"
#include "inet/physicallayer/contract/packetlevel/SignalTag_m.h"

#ifdef NS3_VALIDATION
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#endif

namespace inet {
namespace physicallayer {

Define_Module(Radio);

Radio::~Radio()
{
    // NOTE: can't use the medium module here, because it may have been already deleted
    cModule *medium = getSimulation()->getModule(mediumModuleId);
    if (medium != nullptr)
        check_and_cast<IRadioMedium *>(medium)->removeRadio(this);
    cancelAndDelete(transmissionTimer);
    cancelAndDelete(switchTimer);
}

void Radio::initialize(int stage)
{
    PhysicalLayerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        switchTimer = new cMessage("switchTimer");
        transmissionTimer = new cMessage("transmissionTimer");
        antenna = check_and_cast<IAntenna *>(getSubmodule("antenna"));
        transmitter = check_and_cast<ITransmitter *>(getSubmodule("transmitter"));
        receiver = check_and_cast<IReceiver *>(getSubmodule("receiver"));
        medium = getModuleFromPar<IRadioMedium>(par("radioMediumModule"), this);
        mediumModuleId = check_and_cast<cModule *>(medium)->getId();
        upperLayerIn = gate("upperLayerIn");
        upperLayerOut = gate("upperLayerOut");
        radioIn = gate("radioIn");
        radioIn->setDeliverOnReceptionStart(true);
        sendRawBytes = par("sendRawBytes");
        separateTransmissionParts = par("separateTransmissionParts");
        separateReceptionParts = par("separateReceptionParts");
        WATCH(radioMode);
        WATCH(receptionState);
        WATCH(transmissionState);
        WATCH(receivedSignalPart);
        WATCH(transmittedSignalPart);
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        medium->addRadio(this);
        initializeRadioMode();
        parseRadioModeSwitchingTimes();
    }
    else if (stage == INITSTAGE_LAST) {
        EV_INFO << "Initialized " << getCompleteStringRepresentation() << endl;
    }
}

void Radio::initializeRadioMode() {
    const char *initialRadioMode = par("initialRadioMode");
    if(!strcmp(initialRadioMode, "off"))
        completeRadioModeSwitch(IRadio::RADIO_MODE_OFF);
    else if(!strcmp(initialRadioMode, "sleep"))
        completeRadioModeSwitch(IRadio::RADIO_MODE_SLEEP);
    else if(!strcmp(initialRadioMode, "receiver"))
        completeRadioModeSwitch(IRadio::RADIO_MODE_RECEIVER);
    else if(!strcmp(initialRadioMode, "transmitter"))
        completeRadioModeSwitch(IRadio::RADIO_MODE_TRANSMITTER);
    else if(!strcmp(initialRadioMode, "transceiver"))
        completeRadioModeSwitch(IRadio::RADIO_MODE_TRANSCEIVER);
    else
        throw cRuntimeError("Unknown initialRadioMode");
}

std::ostream& Radio::printToStream(std::ostream& stream, int level) const
{
    stream << static_cast<const cSimpleModule *>(this);
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", antenna = " << printObjectToString(antenna, level + 1)
               << ", transmitter = " << printObjectToString(transmitter, level + 1)
               << ", receiver = " << printObjectToString(receiver, level + 1);
    return stream;
}

void Radio::setRadioMode(RadioMode newRadioMode)
{
    Enter_Method_Silent();
    if (newRadioMode < RADIO_MODE_OFF || newRadioMode > RADIO_MODE_SWITCHING)
        throw cRuntimeError("Unknown radio mode: %d", newRadioMode);
    else if (newRadioMode == RADIO_MODE_SWITCHING)
        throw cRuntimeError("Cannot switch manually to RADIO_MODE_SWITCHING");
    else if (radioMode == RADIO_MODE_SWITCHING || switchTimer->isScheduled())
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
    EV_DETAIL << "Starting to change radio mode from \x1b[1m" << getRadioModeName(radioMode) << "\x1b[0m to \x1b[1m" << getRadioModeName(newRadioMode) << "\x1b[0m." << endl;
    previousRadioMode = radioMode;
    radioMode = RADIO_MODE_SWITCHING;
    nextRadioMode = newRadioMode;
    emit(radioModeChangedSignal, radioMode);
    scheduleAfter(switchingTime, switchTimer);
}

void Radio::completeRadioModeSwitch(RadioMode newRadioMode)
{
    EV_INFO << "Radio mode changed from \x1b[1m" << getRadioModeName(previousRadioMode) << "\x1b[0m to \x1b[1m" << getRadioModeName(newRadioMode) << "\x1b[0m." << endl;
    if (!isReceiverMode(newRadioMode) && receptionTimer != nullptr)
        abortReception(receptionTimer);
    if (!isTransmitterMode(newRadioMode) && transmissionTimer->isScheduled())
        abortTransmission();
    radioMode = previousRadioMode = nextRadioMode = newRadioMode;
    emit(radioModeChangedSignal, newRadioMode);
    updateTransceiverState();
    updateTransceiverPart();
}

const ITransmission *Radio::getTransmissionInProgress() const
{
    if (!transmissionTimer->isScheduled())
        return nullptr;
    else
        return static_cast<WirelessSignal *>(transmissionTimer->getContextPointer())->getTransmission();
}

const ITransmission *Radio::getReceptionInProgress() const
{
    if (receptionTimer == nullptr)
        return nullptr;
    else
        return static_cast<WirelessSignal *>(receptionTimer->getControlInfo())->getTransmission();
}

IRadioSignal::SignalPart Radio::getTransmittedSignalPart() const
{
    return transmittedSignalPart;
}

IRadioSignal::SignalPart Radio::getReceivedSignalPart() const
{
    return receivedSignalPart;
}

void Radio::handleMessageWhenDown(cMessage *message)
{
    if (message->getArrivalGate() == radioIn || isReceptionTimer(message))
        delete message;
    else
        PhysicalLayerBase::handleMessageWhenDown(message);
}

void Radio::handleSelfMessage(cMessage *message)
{
    if (message == switchTimer)
        handleSwitchTimer(message);
    else if (message == transmissionTimer)
        handleTransmissionTimer(message);
    else if (isReceptionTimer(message))
        handleReceptionTimer(message);
    else
        throw cRuntimeError("Unknown self message");
}

void Radio::handleSwitchTimer(cMessage *message)
{
    completeRadioModeSwitch(nextRadioMode);
}

void Radio::handleTransmissionTimer(cMessage *message)
{
    if (message->getKind() == IRadioSignal::SIGNAL_PART_WHOLE)
        endTransmission();
    else if (message->getKind() == IRadioSignal::SIGNAL_PART_PREAMBLE)
        continueTransmission();
    else if (message->getKind() == IRadioSignal::SIGNAL_PART_HEADER)
        continueTransmission();
    else if (message->getKind() == IRadioSignal::SIGNAL_PART_DATA)
        endTransmission();
    else
        throw cRuntimeError("Unknown self message");
}

void Radio::handleReceptionTimer(cMessage *message)
{
    if (message->getKind() == IRadioSignal::SIGNAL_PART_WHOLE)
        endReception(message);
    else if (message->getKind() == IRadioSignal::SIGNAL_PART_PREAMBLE)
        continueReception(message);
    else if (message->getKind() == IRadioSignal::SIGNAL_PART_HEADER)
        continueReception(message);
    else if (message->getKind() == IRadioSignal::SIGNAL_PART_DATA)
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
        delete message;
    }
    else
        throw cRuntimeError("Unsupported command");
}

void Radio::handleLowerCommand(cMessage *message)
{
    throw cRuntimeError("Unsupported command");
}

void Radio::handleUpperPacket(Packet *packet)
{
    emit(packetReceivedFromUpperSignal, packet);
    if (isTransmitterMode(radioMode)) {
        if (transmissionTimer->isScheduled())
            throw cRuntimeError("Received frame from upper layer while already transmitting.");
        if (separateTransmissionParts)
            startTransmission(packet, IRadioSignal::SIGNAL_PART_PREAMBLE);
        else
            startTransmission(packet, IRadioSignal::SIGNAL_PART_WHOLE);
    }
    else {
        EV_ERROR << "Radio is not in transmitter or transceiver mode, dropping frame." << endl;
        delete packet;
    }
}

void Radio::handleSignal(WirelessSignal *signal)
{
    auto receptionTimer = createReceptionTimer(signal);
    if (separateReceptionParts)
        startReception(receptionTimer, IRadioSignal::SIGNAL_PART_PREAMBLE);
    else
        startReception(receptionTimer, IRadioSignal::SIGNAL_PART_WHOLE);
}

void Radio::handleStartOperation(LifecycleOperation *operation)
{
    // NOTE: we ignore radio mode switching during start
    initializeRadioMode();
}

void Radio::handleStopOperation(LifecycleOperation *operation)
{
    // NOTE: we ignore radio mode switching and ongoing transmission during shutdown
    cancelEvent(switchTimer);
    if (transmissionTimer->isScheduled())
        abortTransmission();
    completeRadioModeSwitch(RADIO_MODE_OFF);
}

void Radio::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(switchTimer);
    if (transmissionTimer->isScheduled())
        abortTransmission();
    completeRadioModeSwitch(RADIO_MODE_OFF);
}

void Radio::startTransmission(Packet *macFrame, IRadioSignal::SignalPart part)
{
    auto signal = createSignal(macFrame);
    auto transmission = signal->getTransmission();
    transmissionTimer->setKind(part);
    transmissionTimer->setContextPointer(const_cast<WirelessSignal *>(signal));

#ifdef NS3_VALIDATION
    auto *df = dynamic_cast<inet::ieee80211::Ieee80211DataHeader *>(macFrame);
    const char *ac = "NA";
    if (df != nullptr && df->getType() == inet::ieee80211::ST_DATA_WITH_QOS) {
        switch (df->getTid()) {
            case 1: case 2: ac = "AC_BK"; break;
            case 0: case 3: ac = "AC_BE"; break;
            case 4: case 5: ac = "AC_VI"; break;
            case 6: case 7: ac = "AC_VO"; break;
            default: ac = "???"; break;
        }
    }
    const char *lastSeq = strchr(macFrame->getName(), '-');
    if (lastSeq == nullptr)
        lastSeq = "-1";
    else
        lastSeq++;
    std::cout << "TX: node = " << getId() << ", ac = " << ac << ", seq = " << lastSeq << ", start = " << simTime().inUnit(SIMTIME_PS) << ", duration = " << signal->getDuration().inUnit(SIMTIME_PS) << std::endl;
#endif

    scheduleAt(transmission->getEndTime(part), transmissionTimer);
    EV_INFO << "Transmission started: " << (IWirelessSignal *)signal << " " << IRadioSignal::getSignalPartName(part) << " as " << transmission << endl;
    updateTransceiverState();
    updateTransceiverPart();
    emit(transmissionStartedSignal, check_and_cast<const cObject *>(transmission));
    // TODO: move to radio medium
    check_and_cast<RadioMedium *>(medium)->emit(IRadioMedium::signalDepartureStartedSignal, check_and_cast<const cObject *>(transmission));
}

void Radio::continueTransmission()
{
    auto previousPart = (IRadioSignal::SignalPart)transmissionTimer->getKind();
    auto nextPart = (IRadioSignal::SignalPart)(previousPart + 1);
    auto signal = static_cast<WirelessSignal *>(transmissionTimer->getContextPointer());
    auto transmission = signal->getTransmission();
    EV_INFO << "Transmission ended: " << (IWirelessSignal *)signal << " " << IRadioSignal::getSignalPartName(previousPart) << " as " << signal->getTransmission() << endl;
    transmissionTimer->setKind(nextPart);
    scheduleAt(transmission->getEndTime(nextPart), transmissionTimer);
    EV_INFO << "Transmission started: " << (IWirelessSignal *)signal << " " << IRadioSignal::getSignalPartName(nextPart) << " as " << transmission << endl;
    updateTransceiverState();
    updateTransceiverPart();
}

void Radio::endTransmission()
{
    auto part = (IRadioSignal::SignalPart)transmissionTimer->getKind();
    auto signal = static_cast<WirelessSignal *>(transmissionTimer->getContextPointer());
    auto transmission = signal->getTransmission();
    transmissionTimer->setContextPointer(nullptr);
    EV_INFO << "Transmission ended: " << (IWirelessSignal *)signal << " " << IRadioSignal::getSignalPartName(part) << " as " << transmission << endl;
    updateTransceiverState();
    updateTransceiverPart();
    emit(transmissionEndedSignal, check_and_cast<const cObject *>(transmission));
    // TODO: move to radio medium
    check_and_cast<RadioMedium *>(medium)->emit(IRadioMedium::signalDepartureEndedSignal, check_and_cast<const cObject *>(transmission));
}

void Radio::abortTransmission()
{
    auto part = (IRadioSignal::SignalPart)transmissionTimer->getKind();
    auto signal = static_cast<WirelessSignal *>(transmissionTimer->getContextPointer());
    auto transmission = signal->getTransmission();
    transmissionTimer->setContextPointer(nullptr);
    EV_INFO << "Transmission aborted: " << (IWirelessSignal *)signal << " " << IRadioSignal::getSignalPartName(part) << " as " << transmission << endl;
    EV_WARN << "Aborting ongoing transmissions is not supported" << endl;
    cancelEvent(transmissionTimer);
    updateTransceiverState();
    updateTransceiverPart();
}

WirelessSignal *Radio::createSignal(Packet *packet) const
{
    encapsulate(packet);
    if (sendRawBytes) {
        // TODO: this doesn't always work, because the packet length may not be divisible by 8
        auto rawPacket = new Packet(packet->getName(), packet->peekAllAsBytes());
        rawPacket->copyTags(*packet);
        delete packet;
        packet = rawPacket;
    }
    WirelessSignal *signal = check_and_cast<WirelessSignal *>(medium->transmitPacket(this, packet));
    ASSERT(signal->getDuration() != 0);
    return signal;
}

void Radio::startReception(cMessage *timer, IRadioSignal::SignalPart part)
{
    auto signal = static_cast<WirelessSignal *>(timer->getControlInfo());
    auto arrival = signal->getArrival();
    auto reception = signal->getReception();
// TODO: should be this, but it breaks fingerprints: if (receptionTimer == nullptr && isReceiverMode(radioMode) && arrival->getStartTime(part) == simTime()) {
    if (isReceiverMode(radioMode) && arrival->getStartTime(part) == simTime()) {
        auto transmission = signal->getTransmission();
        auto isReceptionAttempted = medium->isReceptionAttempted(this, transmission, part);
        EV_INFO << "Reception started: " << (isReceptionAttempted ? "\x1b[1mattempting\x1b[0m" : "\x1b[1mnot attempting\x1b[0m") << " " << (IWirelessSignal *)signal << " " << IRadioSignal::getSignalPartName(part) << " as " << reception << endl;
        if (isReceptionAttempted) {
            receptionTimer = timer;
            emit(receptionStartedSignal, check_and_cast<const cObject *>(reception));
        }
    }
    else
        EV_INFO << "Reception started: \x1b[1mignoring\x1b[0m " << (IWirelessSignal *)signal << " " << IRadioSignal::getSignalPartName(part) << " as " << reception << endl;
    timer->setKind(part);
    scheduleAt(arrival->getEndTime(part), timer);
    updateTransceiverState();
    updateTransceiverPart();
    // TODO: move to radio medium
    check_and_cast<RadioMedium *>(medium)->emit(IRadioMedium::signalArrivalStartedSignal, check_and_cast<const cObject *>(reception));
}

void Radio::continueReception(cMessage *timer)
{
    auto previousPart = (IRadioSignal::SignalPart)timer->getKind();
    auto nextPart = (IRadioSignal::SignalPart)(previousPart + 1);
    auto signal = static_cast<WirelessSignal *>(timer->getControlInfo());
    auto arrival = signal->getArrival();
    auto reception = signal->getReception();
    if (timer == receptionTimer && isReceiverMode(radioMode) && arrival->getEndTime(previousPart) == simTime()) {
        auto transmission = signal->getTransmission();
        bool isReceptionSuccessful = medium->isReceptionSuccessful(this, transmission, previousPart);
        EV_INFO << "Reception ended: " << (isReceptionSuccessful ? "\x1b[1msuccessfully\x1b[0m" : "\x1b[1munsuccessfully\x1b[0m") << " for " << (IWirelessSignal *)signal << " " << IRadioSignal::getSignalPartName(previousPart) << " as " << reception << endl;
        if (!isReceptionSuccessful)
            receptionTimer = nullptr;
        auto isReceptionAttempted = medium->isReceptionAttempted(this, transmission, nextPart);
        EV_INFO << "Reception started: " << (isReceptionAttempted ? "\x1b[1mattempting\x1b[0m" : "\x1b[1mnot attempting\x1b[0m") << " " << (IWirelessSignal *)signal << " " << IRadioSignal::getSignalPartName(nextPart) << " as " << reception << endl;
        if (!isReceptionAttempted)
            receptionTimer = nullptr;
        // TODO: FIXME: see handling packets with incorrect PHY headers in the TODO file
    }
    else {
        EV_INFO << "Reception ended: \x1b[1mignoring\x1b[0m " << (IWirelessSignal *)signal << " " << IRadioSignal::getSignalPartName(previousPart) << " as " << reception << endl;
        EV_INFO << "Reception started: \x1b[1mignoring\x1b[0m " << (IWirelessSignal *)signal << " " << IRadioSignal::getSignalPartName(nextPart) << " as " << reception << endl;
    }
    timer->setKind(nextPart);
    scheduleAt(arrival->getEndTime(nextPart), timer);
    updateTransceiverState();
    updateTransceiverPart();
}

void Radio::endReception(cMessage *timer)
{
    auto part = (IRadioSignal::SignalPart)timer->getKind();
    auto signal = static_cast<WirelessSignal *>(timer->getControlInfo());
    auto arrival = signal->getArrival();
    auto reception = signal->getReception();
    if (timer == receptionTimer && isReceiverMode(radioMode) && arrival->getEndTime() == simTime()) {
        auto transmission = signal->getTransmission();
// TODO: this would draw twice from the random number generator in isReceptionSuccessful: auto isReceptionSuccessful = medium->isReceptionSuccessful(this, transmission, part);
        auto isReceptionSuccessful = medium->getReceptionDecision(this, signal->getListening(), transmission, part)->isReceptionSuccessful();
        EV_INFO << "Reception ended: " << (isReceptionSuccessful ? "\x1b[1msuccessfully\x1b[0m" : "\x1b[1munsuccessfully\x1b[0m") << " for " << (IWirelessSignal *)signal << " " << IRadioSignal::getSignalPartName(part) << " as " << reception << endl;
        auto macFrame = medium->receivePacket(this, signal);
        take(macFrame);
        // TODO: FIXME: see handling packets with incorrect PHY headers in the TODO file
        decapsulate(macFrame);
        sendUp(macFrame);
        receptionTimer = nullptr;
        emit(receptionEndedSignal, check_and_cast<const cObject *>(reception));
    }
    else
        EV_INFO << "Reception ended: \x1b[1mignoring\x1b[0m " << (IWirelessSignal *)signal << " " << IRadioSignal::getSignalPartName(part) << " as " << reception << endl;
    updateTransceiverState();
    updateTransceiverPart();
    delete timer;
    // TODO: move to radio medium
    check_and_cast<RadioMedium *>(medium)->emit(IRadioMedium::signalArrivalEndedSignal, check_and_cast<const cObject *>(reception));
}

void Radio::abortReception(cMessage *timer)
{
    auto signal = static_cast<WirelessSignal *>(timer->getControlInfo());
    auto part = (IRadioSignal::SignalPart)timer->getKind();
    auto reception = signal->getReception();
    EV_INFO << "Reception \x1b[1maborted\x1b[0m: for " << (IWirelessSignal *)signal << " " << IRadioSignal::getSignalPartName(part) << " as " << reception << endl;
    if (timer == receptionTimer)
        receptionTimer = nullptr;
    updateTransceiverState();
    updateTransceiverPart();
}

void Radio::captureReception(cMessage *timer)
{
    // TODO: this would be called when the receiver switches to a stronger signal while receiving a weaker one
    throw cRuntimeError("Not yet implemented");
}

void Radio::sendUp(Packet *macFrame)
{
    EV_INFO << "Sending up " << macFrame << endl;
    emit(packetSentToUpperSignal, macFrame);
    send(macFrame, upperLayerOut);
}

cMessage *Radio::createReceptionTimer(WirelessSignal *signal) const
{
    cMessage *timer = new cMessage("receptionTimer");
    timer->setControlInfo(signal);
    return timer;
}

bool Radio::isReceptionTimer(const cMessage *message) const
{
    return !strcmp(message->getName(), "receptionTimer");
}

bool Radio::isReceiverMode(IRadio::RadioMode radioMode) const
{
    return radioMode == RADIO_MODE_RECEIVER || radioMode == RADIO_MODE_TRANSCEIVER;
}

bool Radio::isTransmitterMode(IRadio::RadioMode radioMode) const
{
    return radioMode == RADIO_MODE_TRANSMITTER || radioMode == RADIO_MODE_TRANSCEIVER;
}

bool Radio::isListeningPossible() const
{
    const simtime_t now = simTime();
    const Coord& position = antenna->getMobility()->getCurrentPosition();
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
    else if (receptionTimer && receptionTimer->isScheduled())
        newRadioReceptionState = RECEPTION_STATE_RECEIVING;
    else if (isListeningPossible())
        newRadioReceptionState = RECEPTION_STATE_BUSY;
    else
        newRadioReceptionState = RECEPTION_STATE_IDLE;
    if (receptionState != newRadioReceptionState) {
        EV_INFO << "Changing radio reception state from \x1b[1m" << getRadioReceptionStateName(receptionState) << "\x1b[0m to \x1b[1m" << getRadioReceptionStateName(newRadioReceptionState) << "\x1b[0m." << endl;
        receptionState = newRadioReceptionState;
        emit(receptionStateChangedSignal, newRadioReceptionState);
    }
    // transmission state
    TransmissionState newRadioTransmissionState;
    if (radioMode == RADIO_MODE_OFF || radioMode == RADIO_MODE_SLEEP || radioMode == RADIO_MODE_RECEIVER)
        newRadioTransmissionState = TRANSMISSION_STATE_UNDEFINED;
    else if (transmissionTimer->isScheduled())
        newRadioTransmissionState = TRANSMISSION_STATE_TRANSMITTING;
    else
        newRadioTransmissionState = TRANSMISSION_STATE_IDLE;
    if (transmissionState != newRadioTransmissionState) {
        EV_INFO << "Changing radio transmission state from \x1b[1m" << getRadioTransmissionStateName(transmissionState) << "\x1b[0m to \x1b[1m" << getRadioTransmissionStateName(newRadioTransmissionState) << "\x1b[0m." << endl;
        transmissionState = newRadioTransmissionState;
        emit(transmissionStateChangedSignal, newRadioTransmissionState);
    }
}

void Radio::updateTransceiverPart()
{
    IRadioSignal::SignalPart newReceivedPart = receptionTimer == nullptr ? IRadioSignal::SIGNAL_PART_NONE : (IRadioSignal::SignalPart)receptionTimer->getKind();
    if (receivedSignalPart != newReceivedPart) {
        EV_INFO << "Changing radio received signal part from \x1b[1m" << IRadioSignal::getSignalPartName(receivedSignalPart) << "\x1b[0m to \x1b[1m" << IRadioSignal::getSignalPartName(newReceivedPart) << "\x1b[0m." << endl;
        receivedSignalPart = newReceivedPart;
        emit(receivedSignalPartChangedSignal, receivedSignalPart);
    }
    IRadioSignal::SignalPart newTransmittedPart = !transmissionTimer->isScheduled() ? IRadioSignal::SIGNAL_PART_NONE : (IRadioSignal::SignalPart)transmissionTimer->getKind();
    if (transmittedSignalPart != newTransmittedPart) {
        EV_INFO << "Changing radio transmitted signal part from \x1b[1m" << IRadioSignal::getSignalPartName(transmittedSignalPart) << "\x1b[0m to \x1b[1m" << IRadioSignal::getSignalPartName(newTransmittedPart) << "\x1b[0m." << endl;
        transmittedSignalPart = newTransmittedPart;
        emit(transmittedSignalPartChangedSignal, transmittedSignalPart);
    }
}

} // namespace physicallayer
} // namespace inet

