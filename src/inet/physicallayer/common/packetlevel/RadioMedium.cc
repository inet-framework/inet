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

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/NotifierConsts.h"
#include "inet/linklayer/contract/IMACFrame.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/physicallayer/common/packetlevel/Interference.h"
#include "inet/physicallayer/common/packetlevel/Radio.h"
#include "inet/physicallayer/common/packetlevel/RadioMedium.h"

namespace inet {

namespace physicallayer {

Define_Module(RadioMedium);

RadioMedium::RadioMedium() :
    propagation(nullptr),
    pathLoss(nullptr),
    obstacleLoss(nullptr),
    analogModel(nullptr),
    backgroundNoise(nullptr),
    physicalEnvironment(nullptr),
    material(nullptr),
    rangeFilter(RANGE_FILTER_ANYWHERE),
    radioModeFilter(false),
    listeningFilter(false),
    macAddressFilter(false),
    recordCommunicationLog(false),
    removeNonInterferingTransmissionsTimer(nullptr),
    mediumLimitCache(nullptr),
    neighborCache(nullptr),
    communicationCache(nullptr),
    mediumVisualizer(nullptr),
    transmissionCount(0),
    radioFrameSendCount(0),
    receptionComputationCount(0),
    interferenceComputationCount(0),
    receptionDecisionComputationCount(0),
    listeningDecisionComputationCount(0),
    cacheReceptionGetCount(0),
    cacheReceptionHitCount(0),
    cacheInterferenceGetCount(0),
    cacheInterferenceHitCount(0),
    cacheNoiseGetCount(0),
    cacheNoiseHitCount(0),
    cacheSNIRGetCount(0),
    cacheSNIRHitCount(0),
    cacheDecisionGetCount(0),
    cacheDecisionHitCount(0),
    cacheResultGetCount(0),
    cacheResultHitCount(0)
{
}

RadioMedium::~RadioMedium()
{
    cancelAndDelete(removeNonInterferingTransmissionsTimer);
    for (const auto transmission : transmissions) {
        delete communicationCache->getCachedFrame(transmission);
        delete transmission;
    }
    if (recordCommunicationLog)
        communicationLog.close();
}

void RadioMedium::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        // initialize parameters
        propagation = check_and_cast<IPropagation *>(getSubmodule("propagation"));
        pathLoss = check_and_cast<IPathLoss *>(getSubmodule("pathLoss"));
        obstacleLoss = dynamic_cast<IObstacleLoss *>(getSubmodule("obstacleLoss"));
        analogModel = check_and_cast<IAnalogModel *>(getSubmodule("analogModel"));
        backgroundNoise = dynamic_cast<IBackgroundNoise *>(getSubmodule("backgroundNoise"));
        mediumLimitCache = check_and_cast<IMediumLimitCache *>(getSubmodule("mediumLimitCache"));
        neighborCache = dynamic_cast<INeighborCache *>(getSubmodule("neighborCache"));
        communicationCache = check_and_cast<ICommunicationCache *>(getSubmodule("communicationCache"));
        physicalEnvironment = dynamic_cast<IPhysicalEnvironment *>(getModuleByPath("environment"));
        mediumVisualizer = dynamic_cast<MediumVisualizer *>(getSubmodule("mediumVisualizer"));
        material = physicalEnvironment != nullptr ? physicalEnvironment->getMaterialRegistry()->getMaterial("air") : nullptr;
        const char *rangeFilterString = par("rangeFilter");
        if (!strcmp(rangeFilterString, ""))
            rangeFilter = RANGE_FILTER_ANYWHERE;
        else if (!strcmp(rangeFilterString, "interferenceRange"))
            rangeFilter = RANGE_FILTER_INTERFERENCE_RANGE;
        else if (!strcmp(rangeFilterString, "communicationRange"))
            rangeFilter = RANGE_FILTER_COMMUNICATION_RANGE;
        else
            throw cRuntimeError("Unknown range filter: '%s'", rangeFilter);
        radioModeFilter = par("radioModeFilter");
        listeningFilter = par("listeningFilter");
        macAddressFilter = par("macAddressFilter");
        // initialize timers
        removeNonInterferingTransmissionsTimer = new cMessage("removeNonInterferingTransmissions");
        // initialize logging
        recordCommunicationLog = par("recordCommunicationLog");
        if (recordCommunicationLog)
            communicationLog.open();
    }
    else if (stage == INITSTAGE_LAST)
        EV_INFO << "Initialized " << getCompleteStringRepresentation() << endl;
}

void RadioMedium::finish()
{
    double receptionCacheHitPercentage = 100 * (double)cacheReceptionHitCount / (double)cacheReceptionGetCount;
    double interferenceCacheHitPercentage = 100 * (double)cacheInterferenceHitCount / (double)cacheInterferenceGetCount;
    double noiseCacheHitPercentage = 100 * (double)cacheNoiseHitCount / (double)cacheNoiseGetCount;
    double snirCacheHitPercentage = 100 * (double)cacheSNIRHitCount / (double)cacheSNIRGetCount;
    double decisionCacheHitPercentage = 100 * (double)cacheDecisionHitCount / (double)cacheDecisionGetCount;
    double resultCacheHitPercentage = 100 * (double)cacheResultHitCount / (double)cacheResultGetCount;
    EV_INFO << "Transmission count = " << transmissionCount << endl;
    EV_INFO << "Radio frame send count = " << radioFrameSendCount << endl;
    EV_INFO << "Reception computation count = " << receptionComputationCount << endl;
    EV_INFO << "Interference computation count = " << interferenceComputationCount << endl;
    EV_INFO << "Reception decision computation count = " << receptionDecisionComputationCount << endl;
    EV_INFO << "Listening decision computation count = " << listeningDecisionComputationCount << endl;
    EV_INFO << "Reception cache hit = " << receptionCacheHitPercentage << " %" << endl;
    EV_INFO << "Interference cache hit = " << interferenceCacheHitPercentage << " %" << endl;
    EV_INFO << "Noise cache hit = " << noiseCacheHitPercentage << " %" << endl;
    EV_INFO << "SNIR cache hit = " << snirCacheHitPercentage << " %" << endl;
    EV_INFO << "Reception decision cache hit = " << decisionCacheHitPercentage << " %" << endl;
    EV_INFO << "Reception result cache hit = " << resultCacheHitPercentage << " %" << endl;
    recordScalar("transmission count", transmissionCount);
    recordScalar("radio frame send count", radioFrameSendCount);
    recordScalar("reception computation count", receptionComputationCount);
    recordScalar("interference computation count", interferenceComputationCount);
    recordScalar("reception decision computation count", receptionDecisionComputationCount);
    recordScalar("listening decision computation count", listeningDecisionComputationCount);
    recordScalar("reception cache hit", receptionCacheHitPercentage, "%");
    recordScalar("interference cache hit", interferenceCacheHitPercentage, "%");
    recordScalar("noise cache hit", noiseCacheHitPercentage, "%");
    recordScalar("snir cache hit", snirCacheHitPercentage, "%");
    recordScalar("reception decision cache hit", decisionCacheHitPercentage, "%");
    recordScalar("reception result cache hit", resultCacheHitPercentage, "%");
}

std::ostream& RadioMedium::printToStream(std::ostream &stream, int level) const
{
    stream << static_cast<const cSimpleModule *>(this);
    if (level >= PRINT_LEVEL_TRACE) {
        stream << ", propagation = " << printObjectToString(propagation, level - 1)
               << ", pathLoss = " << printObjectToString(pathLoss, level - 1)
               << ", analogModel = " << printObjectToString(analogModel, level - 1)
               << ", obstacleLoss = " << printObjectToString(obstacleLoss, level - 1)
               << ", backgroundNoise = " << printObjectToString(backgroundNoise, level - 1)
               << ", mediumLimitCache = " << printObjectToString(mediumLimitCache, level - 1)
               << ", neighborCache = " << printObjectToString(neighborCache, level - 1)
               << ", communicationCache = " << printObjectToString(communicationCache, level - 1) ;
    }
    return stream;
}

void RadioMedium::handleMessage(cMessage *message)
{
    if (message == removeNonInterferingTransmissionsTimer) {
        removeNonInterferingTransmissions();
        if (mediumVisualizer != nullptr)
            mediumVisualizer->mediumChanged();
    }
    else
        throw cRuntimeError("Unknown message");
}

bool RadioMedium::isRadioMacAddress(const IRadio *radio, const MACAddress address) const
{
    cModule *host = getContainingNode(const_cast<cModule *>(check_and_cast<const cModule *>(radio)));
    IInterfaceTable *interfaceTable = check_and_cast<IInterfaceTable *>(host->getSubmodule("interfaceTable"));
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        const InterfaceEntry *interface = interfaceTable->getInterface(i);
        if (interface && interface->getMacAddress() == address)
            return true;
    }
    return false;
}

bool RadioMedium::isInCommunicationRange(const ITransmission *transmission, const Coord startPosition, const Coord endPosition) const
{
    m maxCommunicationRange = mediumLimitCache->getMaxCommunicationRange();
    return std::isnan(maxCommunicationRange.get()) ||
           (transmission->getStartPosition().distance(startPosition) < maxCommunicationRange.get() &&
            transmission->getEndPosition().distance(endPosition) < maxCommunicationRange.get());
}

bool RadioMedium::isInInterferenceRange(const ITransmission *transmission, const Coord startPosition, const Coord endPosition) const
{
    m maxInterferenceRange = mediumLimitCache->getMaxInterferenceRange();
    return std::isnan(maxInterferenceRange.get()) ||
           (transmission->getStartPosition().distance(startPosition) < maxInterferenceRange.get() &&
            transmission->getEndPosition().distance(endPosition) < maxInterferenceRange.get());
}

bool RadioMedium::isInterferingTransmission(const ITransmission *transmission, const IListening *listening) const
{
    const IRadio *transmitter = transmission->getTransmitter();
    const IRadio *receiver = listening->getReceiver();
    const IArrival *arrival = getArrival(receiver, transmission);
    const simtime_t& minInterferenceTime = mediumLimitCache->getMinInterferenceTime();
    return transmitter != receiver &&
           arrival->getEndTime() >= listening->getStartTime() + minInterferenceTime &&
           arrival->getStartTime() <= listening->getEndTime() - minInterferenceTime &&
           isInInterferenceRange(transmission, listening->getStartPosition(), listening->getEndPosition());
}

bool RadioMedium::isInterferingTransmission(const ITransmission *transmission, const IReception *reception) const
{
    const IRadio *transmitter = transmission->getTransmitter();
    const IRadio *receiver = reception->getReceiver();
    const IArrival *arrival = getArrival(receiver, transmission);
    const simtime_t& minInterferenceTime = mediumLimitCache->getMinInterferenceTime();
    return transmitter != receiver &&
           arrival->getEndTime() > reception->getStartTime() + minInterferenceTime &&
           arrival->getStartTime() < reception->getEndTime() - minInterferenceTime &&
           isInInterferenceRange(transmission, reception->getStartPosition(), reception->getEndPosition());
}

void RadioMedium::removeNonInterferingTransmissions()
{
    const simtime_t now = simTime();
    size_t transmissionIndex = 0;
    while (transmissionIndex < transmissions.size() && communicationCache->getCachedInterferenceEndTime(transmissions[transmissionIndex]) <= now)
        transmissionIndex++;
    EV_DEBUG << "Removing " << transmissionIndex << " non interfering transmissions\n";
    for (auto it = transmissions.cbegin(); it != transmissions.cbegin() + transmissionIndex; it++) {
        const ITransmission *transmission = *it;
        auto radioFrame = communicationCache->getCachedFrame(transmission);
        if (mediumVisualizer != nullptr)
            mediumVisualizer->removeTransmission(transmission);
        communicationCache->removeCachedFrame(transmission);
        communicationCache->removeTransmission(transmission);
        delete radioFrame;
        delete transmission;
    }
    transmissions.erase(transmissions.begin(), transmissions.begin() + transmissionIndex);
    communicationCache->removeNonInterferingTransmissions();
    if (transmissions.size() > 0)
        scheduleAt(communicationCache->getCachedInterferenceEndTime(transmissions[0]), removeNonInterferingTransmissionsTimer);
}

const std::vector<const IReception *> *RadioMedium::computeInterferingReceptions(const IListening *listening, const std::vector<const ITransmission *> *transmissions) const
{
    const IRadio *radio = listening->getReceiver();
    std::vector<const ITransmission *> *interferingTransmissions = communicationCache->computeInterferingTransmissions(radio, listening->getStartTime(), listening->getEndTime());
    std::vector<const IReception *> *interferingReceptions = new std::vector<const IReception *>();
    for (const auto interferingTransmission : *interferingTransmissions)
        if (isInterferingTransmission(interferingTransmission, listening))
            interferingReceptions->push_back(getReception(radio, interferingTransmission));
    delete interferingTransmissions;
    return interferingReceptions;
}

const std::vector<const IReception *> *RadioMedium::computeInterferingReceptions(const IReception *reception, const std::vector<const ITransmission *> *transmissions) const
{
    const IRadio *radio = reception->getReceiver();
    const ITransmission *transmission = reception->getTransmission();
    std::vector<const ITransmission *> *interferingTransmissions = communicationCache->computeInterferingTransmissions(radio, reception->getStartTime(), reception->getEndTime());
    std::vector<const IReception *> *interferingReceptions = new std::vector<const IReception *>();
    for (const auto interferingTransmission : *interferingTransmissions)
        if (transmission != interferingTransmission && isInterferingTransmission(interferingTransmission, reception))
            interferingReceptions->push_back(getReception(radio, interferingTransmission));
    delete interferingTransmissions;
    return interferingReceptions;
}

const IReception *RadioMedium::computeReception(const IRadio *radio, const ITransmission *transmission) const
{
    receptionComputationCount++;
    return analogModel->computeReception(radio, transmission, getArrival(radio, transmission));
}

const IInterference *RadioMedium::computeInterference(const IRadio *receiver, const IListening *listening, const std::vector<const ITransmission *> *transmissions) const
{
    interferenceComputationCount++;
    const INoise *noise = backgroundNoise ? backgroundNoise->computeNoise(listening) : nullptr;
    const std::vector<const IReception *> *interferingReceptions = computeInterferingReceptions(listening, transmissions);
    return new Interference(noise, interferingReceptions);
}

const IInterference *RadioMedium::computeInterference(const IRadio *receiver, const IListening *listening, const ITransmission *transmission, const std::vector<const ITransmission *> *transmissions) const
{
    interferenceComputationCount++;
    const IReception *reception = getReception(receiver, transmission);
    const INoise *noise = backgroundNoise ? backgroundNoise->computeNoise(listening) : nullptr;
    const std::vector<const IReception *> *interferingReceptions = computeInterferingReceptions(reception, transmissions);
    return new Interference(noise, interferingReceptions);
}

const IReceptionDecision *RadioMedium::computeReceptionDecision(const IRadio *radio, const IListening *listening, const ITransmission *transmission, IRadioSignal::SignalPart part, const std::vector<const ITransmission *> *transmissions) const
{
    receptionDecisionComputationCount++;
    const IReception *reception = getReception(radio, transmission);
    const IInterference *interference = getInterference(radio, listening, transmission);
    const ISNIR *snir = getSNIR(radio, transmission);
    return radio->getReceiver()->computeReceptionDecision(listening, reception, part, interference, snir);
}

const IReceptionResult *RadioMedium::computeReceptionResult(const IRadio *radio, const IListening *listening, const ITransmission *transmission, const std::vector<const ITransmission *> *transmissions) const
{
    receptionResultComputationCount++;
    const IReception *reception = getReception(radio, transmission);
    const IInterference *interference = getInterference(radio, listening, transmission);
    const ISNIR *snir = getSNIR(radio, transmission);
    return radio->getReceiver()->computeReceptionResult(listening, reception, interference, snir);
}

const IListeningDecision *RadioMedium::computeListeningDecision(const IRadio *radio, const IListening *listening, const std::vector<const ITransmission *> *transmissions) const
{
    listeningDecisionComputationCount++;
    const IInterference *interference = computeInterference(radio, listening, transmissions);
    const IListeningDecision *decision = radio->getReceiver()->computeListeningDecision(listening, interference);
    delete interference;
    return decision;
}

const IArrival *RadioMedium::getArrival(const IRadio *receiver, const ITransmission *transmission) const
{
    return communicationCache->getCachedArrival(receiver, transmission);
}

const IListening *RadioMedium::getListening(const IRadio *receiver, const ITransmission *transmission) const
{
    return communicationCache->getCachedListening(receiver, transmission);
}

const IReception *RadioMedium::getReception(const IRadio *receiver, const ITransmission *transmission) const
{
    cacheReceptionGetCount++;
    const IReception *reception = communicationCache->getCachedReception(receiver, transmission);
    if (reception)
        cacheReceptionHitCount++;
    else {
        reception = computeReception(receiver, transmission);
        communicationCache->setCachedReception(receiver, transmission, reception);
        EV_DEBUG << "Receiving " << transmission << " from medium by " << receiver << " arrives as " << reception << endl;
    }
    return reception;
}

const IInterference *RadioMedium::getInterference(const IRadio *receiver, const ITransmission *transmission) const
{
    return getInterference(receiver, communicationCache->getCachedListening(receiver, transmission), transmission);
}

const IInterference *RadioMedium::getInterference(const IRadio *receiver, const IListening *listening, const ITransmission *transmission) const
{
    cacheInterferenceGetCount++;
    const IInterference *interference = communicationCache->getCachedInterference(receiver, transmission);
    if (interference)
        cacheInterferenceHitCount++;
    else {
        interference = computeInterference(receiver, listening, transmission, const_cast<const std::vector<const ITransmission *> *>(&transmissions));
        communicationCache->setCachedInterference(receiver, transmission, interference);
    }
    return interference;
}

const INoise *RadioMedium::getNoise(const IRadio *receiver, const ITransmission *transmission) const
{
    cacheNoiseGetCount++;
    const INoise *noise = communicationCache->getCachedNoise(receiver, transmission);
    if (noise)
        cacheNoiseHitCount++;
    else {
        const IListening *listening = communicationCache->getCachedListening(receiver, transmission);
        const IInterference *interference = getInterference(receiver, transmission);
        noise = analogModel->computeNoise(listening, interference);
        communicationCache->setCachedNoise(receiver, transmission, noise);
    }
    return noise;
}

const ISNIR *RadioMedium::getSNIR(const IRadio *receiver, const ITransmission *transmission) const
{
    cacheSNIRGetCount++;
    const ISNIR *snir = communicationCache->getCachedSNIR(receiver, transmission);
    if (snir)
        cacheSNIRHitCount++;
    else {
        const IReception *reception = getReception(receiver, transmission);
        const INoise *noise = getNoise(receiver, transmission);
        snir = analogModel->computeSNIR(reception, noise);
        communicationCache->setCachedSNIR(receiver, transmission, snir);
    }
    return snir;
}

const IReceptionDecision *RadioMedium::getReceptionDecision(const IRadio *radio, const IListening *listening, const ITransmission *transmission, IRadioSignal::SignalPart part) const
{
    cacheDecisionGetCount++;
    const IReceptionDecision *decision = communicationCache->getCachedReceptionDecision(radio, transmission, part);
    if (decision)
        cacheDecisionHitCount++;
    else {
        decision = computeReceptionDecision(radio, listening, transmission, part, const_cast<const std::vector<const ITransmission *> *>(&transmissions));
        communicationCache->setCachedReceptionDecision(radio, transmission, part, decision);
        EV_DEBUG << "Receiving " << transmission << " from medium by " << radio << " arrives as " << decision->getReception() << " and results in " << decision << endl;
    }
    return decision;
}

const IReceptionResult *RadioMedium::getReceptionResult(const IRadio *radio, const IListening *listening, const ITransmission *transmission) const
{
    cacheResultGetCount++;
    const IReceptionResult *result = communicationCache->getCachedReceptionResult(radio, transmission);
    if (result)
        cacheResultHitCount++;
    else {
        result = computeReceptionResult(radio, listening, transmission, const_cast<const std::vector<const ITransmission *> *>(&transmissions));
        communicationCache->setCachedReceptionResult(radio, transmission, result);
        EV_DEBUG << "Receiving " << transmission << " from medium by " << radio << " arrives as " << result->getReception() << " and results in " << result << endl;
    }
    return result;
}

void RadioMedium::addRadio(const IRadio *radio)
{
    radios.push_back(radio);
    communicationCache->addRadio(radio);
    if (neighborCache)
        neighborCache->addRadio(radio);
    mediumLimitCache->addRadio(radio);
    for (const auto transmission : transmissions) {
        const IArrival *arrival = propagation->computeArrival(transmission, radio->getAntenna()->getMobility());
        const IListening *listening = radio->getReceiver()->createListening(radio, arrival->getStartTime(), arrival->getEndTime(), arrival->getStartPosition(), arrival->getEndPosition());
        communicationCache->setCachedArrival(radio, transmission, arrival);
        communicationCache->setCachedListening(radio, transmission, listening);
    }
    cModule *radioModule = const_cast<cModule *>(check_and_cast<const cModule *>(radio));
    if (radioModeFilter)
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
    if (listeningFilter)
        radioModule->subscribe(IRadio::listeningChangedSignal, this);
    if (macAddressFilter)
        getContainingNode(radioModule)->subscribe(NF_INTERFACE_CONFIG_CHANGED, this);
}

void RadioMedium::removeRadio(const IRadio *radio)
{
    int radioIndex = radio->getId() - radios[0]->getId();
    radios[radioIndex] = nullptr;
    int radioCount = 0;
    while (radios[radioCount] == nullptr && radioCount < (int)radios.size())
        radioCount++;
    if (radioCount != 0)
        radios.erase(radios.begin(), radios.begin() + radioCount);
    communicationCache->removeRadio(radio);
    if (neighborCache)
        neighborCache->removeRadio(radio);
    mediumLimitCache->removeRadio(radio);
    cModule *radioModule = const_cast<cModule *>(check_and_cast<const cModule *>(radio));
    if (radioModeFilter)
        radioModule->unsubscribe(IRadio::radioModeChangedSignal, this);
    if (listeningFilter)
        radioModule->unsubscribe(IRadio::listeningChangedSignal, this);
    if (macAddressFilter)
        getContainingNode(radioModule)->unsubscribe(NF_INTERFACE_CONFIG_CHANGED, this);
}

void RadioMedium::addTransmission(const IRadio *transmitterRadio, const ITransmission *transmission)
{
    transmissionCount++;
    transmissions.push_back(transmission);
    communicationCache->addTransmission(transmission);
    simtime_t maxArrivalEndTime = simTime();
    for (const auto receiverRadio : radios) {
        if (receiverRadio != nullptr && receiverRadio != transmitterRadio) {
            const IArrival *arrival = propagation->computeArrival(transmission, receiverRadio->getAntenna()->getMobility());
            const Interval *interval = new Interval(arrival->getStartTime(), arrival->getEndTime(), (void *)transmission);
            const IListening *listening = receiverRadio->getReceiver()->createListening(receiverRadio, arrival->getStartTime(), arrival->getEndTime(), arrival->getStartPosition(), arrival->getEndPosition());
            const simtime_t arrivalEndTime = arrival->getEndTime();
            if (arrivalEndTime > maxArrivalEndTime)
                maxArrivalEndTime = arrivalEndTime;
            communicationCache->setCachedArrival(receiverRadio, transmission, arrival);
            communicationCache->setCachedInterval(receiverRadio, transmission, interval);
            communicationCache->setCachedListening(receiverRadio, transmission, listening);
        }
    }
    communicationCache->setCachedInterferenceEndTime(transmission, maxArrivalEndTime + mediumLimitCache->getMaxTransmissionDuration());
    if (!removeNonInterferingTransmissionsTimer->isScheduled()) {
        Enter_Method_Silent();
        scheduleAt(communicationCache->getCachedInterferenceEndTime(transmissions[0]), removeNonInterferingTransmissionsTimer);
    }
    if (mediumVisualizer != nullptr)
        mediumVisualizer->addTransmission(transmission);
}

IRadioFrame *RadioMedium::createTransmitterRadioFrame(const IRadio *radio, cPacket *macFrame)
{
    Enter_Method_Silent();
    take(macFrame);
    auto transmission = radio->getTransmitter()->createTransmission(radio, macFrame, simTime());
    auto radioFrame = new RadioFrame(transmission);
    auto phyFrame = const_cast<cPacket *>(transmission->getPhyFrame());
    auto encapsulatedFrame = phyFrame != nullptr ? phyFrame : macFrame;
    radioFrame->setName(encapsulatedFrame->getName());
    radioFrame->setDuration(transmission->getDuration());
    radioFrame->encapsulate(encapsulatedFrame);
    return radioFrame;
}

IRadioFrame *RadioMedium::createReceiverRadioFrame(const ITransmission *transmission)
{
    auto radioFrame = new RadioFrame(transmission);
    auto phyFrame = const_cast<cPacket *>(transmission->getPhyFrame());
    auto macFrame = const_cast<cPacket *>(transmission->getMacFrame());
    auto encapsulatedFrame = phyFrame != nullptr ? phyFrame : macFrame;
    radioFrame->setName(encapsulatedFrame->getName());
    radioFrame->setDuration(transmission->getDuration());
    radioFrame->encapsulate(encapsulatedFrame->dup());
    return radioFrame;
}

void RadioMedium::sendToAffectedRadios(IRadio *radio, const IRadioFrame *frame)
{
    const RadioFrame *radioFrame = check_and_cast<const RadioFrame *>(frame);
    EV_DEBUG << "Sending " << frame << " with " << radioFrame->getBitLength() << " bits in " << radioFrame->getDuration() * 1E+6 << " us transmission duration"
             << " from " << radio << " on " << (IRadioMedium *)this << "." << endl;
    if (neighborCache && rangeFilter != RANGE_FILTER_ANYWHERE)
    {
        double range;
        if (rangeFilter == RANGE_FILTER_COMMUNICATION_RANGE)
            range = mediumLimitCache->getMaxCommunicationRange(radio).get();
        else if (rangeFilter == RANGE_FILTER_INTERFERENCE_RANGE)
            range = mediumLimitCache->getMaxInterferenceRange(radio).get();
        else
            throw cRuntimeError("Unknown range filter %d", rangeFilter);
        if (std::isnan(range))
        {
            EV_WARN << "We can't use the NeighborCache for radio " << radio->getId() << ": range is NaN" << endl;
            sendToAllRadios(radio, frame);
        }
        else
            neighborCache->sendToNeighbors(radio, frame, range);
    }
    else
        sendToAllRadios(radio, frame);

}

void RadioMedium::sendToRadio(IRadio *transmitter, const IRadio *receiver, const IRadioFrame *frame)
{
    const Radio *transmitterRadio = check_and_cast<const Radio *>(transmitter);
    const Radio *receiverRadio = check_and_cast<const Radio *>(receiver);
    const ITransmission *transmission = frame->getTransmission();
    if (receiverRadio != transmitterRadio && isPotentialReceiver(receiverRadio, transmission)) {
        const IArrival *arrival = getArrival(receiverRadio, transmission);
        simtime_t propagationTime = arrival->getStartPropagationTime();
        EV_DEBUG << "Sending " << frame
                 << " from " << (IRadio *)transmitterRadio << " at " << transmission->getStartPosition()
                 << " to " << (IRadio *)receiverRadio << " at " << arrival->getStartPosition()
                 << " in " << propagationTime * 1E+6 << " us propagation time." << endl;
        auto radioFrame = static_cast<RadioFrame *>(createReceiverRadioFrame(transmission));
        cGate *gate = receiverRadio->getRadioGate()->getPathStartGate();
        ASSERT(dynamic_cast<IRadio *>(getSimulation()->getContextModule()) != nullptr);
        const_cast<Radio *>(transmitterRadio)->sendDirect(radioFrame, propagationTime, transmission->getDuration(), gate);
        communicationCache->setCachedFrame(receiverRadio, transmission, radioFrame);
        radioFrameSendCount++;
    }
}

IRadioFrame *RadioMedium::transmitPacket(const IRadio *radio, cPacket *macFrame)
{
    auto radioFrame = createTransmitterRadioFrame(radio, macFrame);
    auto transmission = radioFrame->getTransmission();
    addTransmission(radio, transmission);
    if (recordCommunicationLog)
        communicationLog.writeTransmission(radio, radioFrame);
    sendToAffectedRadios(const_cast<IRadio *>(radio), radioFrame);
    communicationCache->setCachedFrame(transmission, radioFrame);
    return radioFrame;
}

cPacket *RadioMedium::receivePacket(const IRadio *radio, IRadioFrame *radioFrame)
{
    const ITransmission *transmission = radioFrame->getTransmission();
    const IListening *listening = communicationCache->getCachedListening(radio, transmission);
    if (recordCommunicationLog)
        communicationLog.writeReception(radio, radioFrame);
    const IReceptionResult *result = getReceptionResult(radio, listening, transmission);
    communicationCache->removeCachedReceptionResult(radio, transmission);
    cPacket *macFrame = const_cast<cPacket *>(result->getMacFrame()->dup());
    macFrame->setControlInfo(const_cast<ReceptionIndication *>(result->getIndication()));
    if (mediumVisualizer != nullptr)
        mediumVisualizer->receivePacket(result);
    delete result;
    return macFrame;
}

const IListeningDecision *RadioMedium::listenOnMedium(const IRadio *radio, const IListening *listening) const
{
    const IListeningDecision *decision = computeListeningDecision(radio, listening, const_cast<const std::vector<const ITransmission *> *>(&transmissions));
    EV_DEBUG << "Listening with " << listening << " on medium by " << radio << " results in " << decision << endl;
    return decision;
}

bool RadioMedium::isPotentialReceiver(const IRadio *radio, const ITransmission *transmission) const
{
    const Radio *receiverRadio = check_and_cast<const Radio *>(radio);
    if (radioModeFilter && receiverRadio->getRadioMode() != IRadio::RADIO_MODE_RECEIVER && receiverRadio->getRadioMode() != IRadio::RADIO_MODE_TRANSCEIVER)
        return false;
    else if (listeningFilter && !radio->getReceiver()->computeIsReceptionPossible(getListening(radio, transmission), transmission))
        return false;
    else if (macAddressFilter && !isRadioMacAddress(radio, check_and_cast<const IMACFrame *>(transmission->getMacFrame())->getDestinationAddress()))
        return false;
    else if (rangeFilter == RANGE_FILTER_INTERFERENCE_RANGE) {
        const IArrival *arrival = getArrival(radio, transmission);
        return isInInterferenceRange(transmission, arrival->getStartPosition(), arrival->getEndPosition());
    }
    else if (rangeFilter == RANGE_FILTER_COMMUNICATION_RANGE) {
        const IArrival *arrival = getArrival(radio, transmission);
        return isInCommunicationRange(transmission, arrival->getStartPosition(), arrival->getEndPosition());
    }
    else
        return true;
}

bool RadioMedium::isReceptionPossible(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part) const
{
    const IReception *reception = getReception(receiver, transmission);
    const IListening *listening = getListening(receiver, transmission);
    // TODO: why compute?
    const IInterference *interference = computeInterference(receiver, listening, transmission, const_cast<const std::vector<const ITransmission *> *>(&transmissions));
    bool isReceptionPossible = receiver->getReceiver()->computeIsReceptionAttempted(listening, reception, part, interference);
    delete interference;
    if (mediumVisualizer != nullptr)
        mediumVisualizer->mediumChanged();
    return isReceptionPossible;
}

bool RadioMedium::isReceptionAttempted(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part) const
{
    const IReception *reception = getReception(receiver, transmission);
    const IListening *listening = getListening(receiver, transmission);
    // TODO: why compute?
    const IInterference *interference = computeInterference(receiver, listening, transmission, const_cast<const std::vector<const ITransmission *> *>(&transmissions));
    bool isReceptionAttempted = receiver->getReceiver()->computeIsReceptionAttempted(listening, reception, part, interference);
    delete interference;
    if (mediumVisualizer != nullptr)
        mediumVisualizer->mediumChanged();
    return isReceptionAttempted;
}

bool RadioMedium::isReceptionSuccessful(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part) const
{
    const IReception *reception = getReception(receiver, transmission);
    const IListening *listening = getListening(receiver, transmission);
    // TODO: why compute?
    const IInterference *interference = computeInterference(receiver, listening, transmission, const_cast<const std::vector<const ITransmission *> *>(&transmissions));
    const ISNIR *snir = getSNIR(receiver, transmission);
    bool isReceptionSuccessful = receiver->getReceiver()->computeIsReceptionSuccessful(listening, reception, part, interference, snir);
    delete interference;
    if (mediumVisualizer != nullptr)
        mediumVisualizer->mediumChanged();
    return isReceptionSuccessful;
}

void RadioMedium::sendToAllRadios(IRadio *transmitter, const IRadioFrame *frame)
{
    for (const auto radio : radios)
        if (radio != nullptr)
            sendToRadio(transmitter, radio, frame);
}

void RadioMedium::receiveSignal(cComponent *source, simsignal_t signal, long value DETAILS_ARG)
{
    if (signal == IRadio::radioModeChangedSignal || signal == IRadio::listeningChangedSignal || signal == NF_INTERFACE_CONFIG_CHANGED) {
        const Radio *receiverRadio = check_and_cast<const Radio *>(source);
        for (const auto transmission : transmissions) {
            const Radio *transmitterRadio = check_and_cast<const Radio *>(transmission->getTransmitter());
            if (signal == IRadio::listeningChangedSignal) {
                const IArrival *arrival = getArrival(receiverRadio, transmission);
                const IListening *listening = receiverRadio->getReceiver()->createListening(receiverRadio, arrival->getStartTime(), arrival->getEndTime(), arrival->getStartPosition(), arrival->getEndPosition());
                delete communicationCache->getCachedListening(receiverRadio, transmission);
                communicationCache->setCachedListening(receiverRadio, transmission, listening);
            }
            if (communicationCache->getCachedFrame(receiverRadio, transmission) == nullptr &&
                receiverRadio != transmitterRadio && isPotentialReceiver(receiverRadio, transmission))
            {
                const IArrival *arrival = getArrival(receiverRadio, transmission);
                if (arrival->getEndTime() >= simTime()) {
                    cMethodCallContextSwitcher contextSwitcher(transmitterRadio);
                    contextSwitcher.methodCallSilent();
                    const cPacket *macFrame = transmission->getMacFrame();
                    EV_DEBUG << "Picking up " << macFrame << " originally sent "
                             << " from " << (IRadio *)transmitterRadio << " at " << transmission->getStartPosition()
                             << " to " << (IRadio *)receiverRadio << " at " << arrival->getStartPosition()
                             << " in " << arrival->getStartPropagationTime() * 1E+6 << " us propagation time." << endl;
                    auto radioFrame = static_cast<RadioFrame *>(createReceiverRadioFrame(transmission));
                    simtime_t delay = arrival->getStartTime() - simTime();
                    simtime_t duration = delay > 0 ? radioFrame->getDuration() : radioFrame->getDuration() + delay;
                    cGate *gate = receiverRadio->getRadioGate()->getPathStartGate();
                    ASSERT(dynamic_cast<IRadio *>(getSimulation()->getContextModule()) != nullptr);
                    const_cast<Radio *>(transmitterRadio)->sendDirect(radioFrame, delay > 0 ? delay : 0, duration, gate);
                    communicationCache->setCachedFrame(receiverRadio, transmission, radioFrame);
                    radioFrameSendCount++;
                }
            }
        }
    }
}

} // namespace physicallayer

} // namespace inet

