//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/packet/chunk/FieldsChunk.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/physicallayer/common/bitlevel/LayeredTransmission.h"
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
    transmissionCount(0),
    signalSendCount(0),
    receptionComputationCount(0),
    interferenceComputationCount(0),
    receptionDecisionComputationCount(0),
    receptionResultComputationCount(0),
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
    communicationCache->mapTransmissions([&] (const ITransmission *transmission) {
        delete communicationCache->getCachedSignal(transmission);
        delete transmission;
    });
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
        physicalEnvironment = dynamic_cast<physicalenvironment::IPhysicalEnvironment *>(getModuleByPath(par("physicalEnvironmentModule")));
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
    EV_INFO << "Signal send count = " << signalSendCount << endl;
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
    recordScalar("signal send count", signalSendCount);
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
    if (level <= PRINT_LEVEL_TRACE) {
        stream << ", propagation = " << printObjectToString(propagation, level + 1)
               << ", pathLoss = " << printObjectToString(pathLoss, level + 1)
               << ", analogModel = " << printObjectToString(analogModel, level + 1)
               << ", obstacleLoss = " << printObjectToString(obstacleLoss, level + 1)
               << ", backgroundNoise = " << printObjectToString(backgroundNoise, level + 1)
               << ", mediumLimitCache = " << printObjectToString(mediumLimitCache, level + 1)
               << ", neighborCache = " << printObjectToString(neighborCache, level + 1)
               << ", communicationCache = " << printObjectToString(communicationCache, level + 1) ;
    }
    return stream;
}

void RadioMedium::handleMessage(cMessage *message)
{
    if (message == removeNonInterferingTransmissionsTimer)
        removeNonInterferingTransmissions();
    else
        throw cRuntimeError("Unknown message");
}

bool RadioMedium::matchesMacAddressFilter(const IRadio *radio, const Packet *packet) const
{
    auto macAddressReq = const_cast<Packet *>(packet)->findTag<MacAddressInd>();
    if (macAddressReq == nullptr)
        return true;
    const MacAddress address = macAddressReq->getDestAddress();
    if (address.isBroadcast() || address.isMulticast())
        return true;
    cModule *host = getContainingNode(check_and_cast<const cModule *>(radio));
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
    const IRadio *receiver = listening->getReceiver();
    const IArrival *arrival = getArrival(receiver, transmission);
    const simtime_t& minInterferenceTime = mediumLimitCache->getMinInterferenceTime();
    return transmission->getTransmitterId() != receiver->getId() &&
           arrival->getEndTime() >= listening->getStartTime() + minInterferenceTime &&
           arrival->getStartTime() <= listening->getEndTime() - minInterferenceTime &&
           isInInterferenceRange(transmission, listening->getStartPosition(), listening->getEndPosition());
}

bool RadioMedium::isInterferingTransmission(const ITransmission *transmission, const IReception *reception) const
{
    const IRadio *receiver = reception->getReceiver();
    const IArrival *arrival = getArrival(receiver, transmission);
    const simtime_t& minInterferenceTime = mediumLimitCache->getMinInterferenceTime();
    return transmission->getTransmitterId() != receiver->getId() &&
           arrival->getEndTime() > reception->getStartTime() + minInterferenceTime &&
           arrival->getStartTime() < reception->getEndTime() - minInterferenceTime &&
           isInInterferenceRange(transmission, reception->getStartPosition(), reception->getEndPosition());
}

void RadioMedium::removeNonInterferingTransmissions()
{
    communicationCache->removeNonInterferingTransmissions([&] (const ITransmission *transmission) {
        const ISignal *signal = communicationCache->getCachedSignal(transmission);
        emit(signalRemovedSignal, check_and_cast<const cObject *>(transmission));
        delete signal;
        delete transmission;
    });
    communicationCache->mapTransmissions([&] (const ITransmission *transmission) {
        auto interferenceEndTime = communicationCache->getCachedInterferenceEndTime(transmission);
        if (!removeNonInterferingTransmissionsTimer->isScheduled() && interferenceEndTime > simTime())
            scheduleAt(interferenceEndTime + 1E-6, removeNonInterferingTransmissionsTimer);
    });
}

const std::vector<const IReception *> *RadioMedium::computeInterferingReceptions(const IListening *listening) const
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

const std::vector<const IReception *> *RadioMedium::computeInterferingReceptions(const IReception *reception) const
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

const IInterference *RadioMedium::computeInterference(const IRadio *receiver, const IListening *listening) const
{
    interferenceComputationCount++;
    const INoise *noise = backgroundNoise ? backgroundNoise->computeNoise(listening) : nullptr;
    const std::vector<const IReception *> *interferingReceptions = computeInterferingReceptions(listening);
    return new Interference(noise, interferingReceptions);
}

const IInterference *RadioMedium::computeInterference(const IRadio *receiver, const IListening *listening, const ITransmission *transmission) const
{
    interferenceComputationCount++;
    const IReception *reception = getReception(receiver, transmission);
    const INoise *noise = backgroundNoise ? backgroundNoise->computeNoise(listening) : nullptr;
    const std::vector<const IReception *> *interferingReceptions = computeInterferingReceptions(reception);
    return new Interference(noise, interferingReceptions);
}

const IReceptionDecision *RadioMedium::computeReceptionDecision(const IRadio *radio, const IListening *listening, const ITransmission *transmission, IRadioSignal::SignalPart part) const
{
    receptionDecisionComputationCount++;
    const IReception *reception = getReception(radio, transmission);
    const IInterference *interference = getInterference(radio, listening, transmission);
    const ISnir *snir = getSNIR(radio, transmission);
    return radio->getReceiver()->computeReceptionDecision(listening, reception, part, interference, snir);
}

const IReceptionResult *RadioMedium::computeReceptionResult(const IRadio *radio, const IListening *listening, const ITransmission *transmission) const
{
    receptionResultComputationCount++;
    const IReception *reception = getReception(radio, transmission);
    const IInterference *interference = getInterference(radio, listening, transmission);
    const ISnir *snir = getSNIR(radio, transmission);
    const IReceptionDecision *receptionDecision = getReceptionDecision(radio, listening, transmission, IRadioSignal::SIGNAL_PART_WHOLE);
    const std::vector<const IReceptionDecision *> *receptionDecisions = new std::vector<const IReceptionDecision *> {receptionDecision};
    return radio->getReceiver()->computeReceptionResult(listening, reception, interference, snir, receptionDecisions);
}

const IListeningDecision *RadioMedium::computeListeningDecision(const IRadio *radio, const IListening *listening) const
{
    listeningDecisionComputationCount++;
    const IInterference *interference = computeInterference(radio, listening);
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
        interference = computeInterference(receiver, listening, transmission);
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

const ISnir *RadioMedium::getSNIR(const IRadio *receiver, const ITransmission *transmission) const
{
    cacheSNIRGetCount++;
    const ISnir *snir = communicationCache->getCachedSNIR(receiver, transmission);
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
        decision = computeReceptionDecision(radio, listening, transmission, part);
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
        result = computeReceptionResult(radio, listening, transmission);
        communicationCache->setCachedReceptionResult(radio, transmission, result);
        EV_DEBUG << "Receiving " << transmission << " from medium by " << radio << " arrives as " << result->getReception() << " and results in " << result << endl;
    }
    return result;
}

void RadioMedium::addRadio(const IRadio *radio)
{
    Enter_Method("addRadio");
    communicationCache->addRadio(radio);
    if (neighborCache)
        neighborCache->addRadio(radio);
    mediumLimitCache->addRadio(radio);
    communicationCache->mapTransmissions([&] (const ITransmission *transmission) {
        const IArrival *arrival = propagation->computeArrival(transmission, radio->getAntenna()->getMobility());
        const IListening *listening = radio->getReceiver()->createListening(radio, arrival->getStartTime(), arrival->getEndTime(), arrival->getStartPosition(), arrival->getEndPosition());
        communicationCache->setCachedArrival(radio, transmission, arrival);
        communicationCache->setCachedListening(radio, transmission, listening);
    });
    cModule *radioModule = const_cast<cModule *>(check_and_cast<const cModule *>(radio));
    if (radioModeFilter)
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
    if (listeningFilter)
        radioModule->subscribe(IRadio::listeningChangedSignal, this);
    if (macAddressFilter)
        getContainingNode(radioModule)->subscribe(interfaceConfigChangedSignal, this);
    emit(radioAddedSignal, radioModule);
}

void RadioMedium::removeRadio(const IRadio *radio)
{
    Enter_Method("removeRadio");
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
        getContainingNode(radioModule)->unsubscribe(interfaceConfigChangedSignal, this);
    emit(radioRemovedSignal, radioModule);
}

const IRadio *RadioMedium::getRadio(int radioId) const
{
    return communicationCache->getRadio(radioId);
}

void RadioMedium::addTransmission(const IRadio *transmitterRadio, const ITransmission *transmission)
{
    Enter_Method("addTransmission");
    transmissionCount++;
    communicationCache->addTransmission(transmission);
    simtime_t maxArrivalEndTime = transmission->getEndTime();
    communicationCache->mapRadios([&] (const IRadio *receiverRadio) {
        if (receiverRadio != nullptr && receiverRadio != transmitterRadio && receiverRadio->getReceiver() != nullptr) {
            const IArrival *arrival = propagation->computeArrival(transmission, receiverRadio->getAntenna()->getMobility());
            const IntervalTree::Interval *interval = new IntervalTree::Interval(arrival->getStartTime(), arrival->getEndTime(), (void *)transmission);
            const IListening *listening = receiverRadio->getReceiver()->createListening(receiverRadio, arrival->getStartTime(), arrival->getEndTime(), arrival->getStartPosition(), arrival->getEndPosition());
            const simtime_t arrivalEndTime = arrival->getEndTime();
            if (arrivalEndTime > maxArrivalEndTime)
                maxArrivalEndTime = arrivalEndTime;
            communicationCache->setCachedArrival(receiverRadio, transmission, arrival);
            communicationCache->setCachedInterval(receiverRadio, transmission, interval);
            communicationCache->setCachedListening(receiverRadio, transmission, listening);
        }
    });
    communicationCache->setCachedInterferenceEndTime(transmission, maxArrivalEndTime + mediumLimitCache->getMaxTransmissionDuration());
    if (!removeNonInterferingTransmissionsTimer->isScheduled())
        scheduleAt(communicationCache->getCachedInterferenceEndTime(transmission), removeNonInterferingTransmissionsTimer);
    emit(signalAddedSignal, check_and_cast<const cObject *>(transmission));
}

void RadioMedium::removeTransmission(const ITransmission *transmission)
{
    Enter_Method("removeTranmsission");
    const ISignal *signal = communicationCache->getCachedSignal(transmission);
    communicationCache->removeTransmission(transmission);
    emit(signalRemovedSignal, check_and_cast<const cObject *>(transmission));
    delete signal;
    delete transmission;
}

ISignal *RadioMedium::createTransmitterSignal(const IRadio *radio, Packet *packet)
{
    if (packet != nullptr)
        take(packet);
    auto transmission = radio->getTransmitter()->createTransmission(radio, packet, simTime());
    auto signal = new Signal(transmission);
    auto duration = transmission->getDuration();
    if (duration > mediumLimitCache->getMaxTransmissionDuration())
        throw cRuntimeError("Maximum transmission duration is exceeded");
    signal->setDuration(duration);
    if (packet != nullptr) {
        signal->setName(packet->getName());
        signal->encapsulate(packet);
    }
    return signal;
}

ISignal *RadioMedium::createReceiverSignal(const ITransmission *transmission)
{
    auto signal = new Signal(transmission);
    signal->setDuration(transmission->getDuration());
    auto transmitterPacket = transmission->getPacket();
    if (transmitterPacket != nullptr) {
        auto receiverPacket = transmitterPacket->dup();
        receiverPacket->clearTags();
        receiverPacket->addTag<PacketProtocolTag>()->setProtocol(transmitterPacket->getTag<PacketProtocolTag>()->getProtocol());
        signal->setName(receiverPacket->getName());
        signal->encapsulate(receiverPacket);
    }
    return signal;
}

void RadioMedium::sendToAffectedRadios(IRadio *radio, const ISignal *transmittedSignal)
{
    const Signal *signal = check_and_cast<const Signal *>(transmittedSignal);
    EV_DEBUG << "Sending " << transmittedSignal << " with " << signal->getBitLength() << " bits in " << signal->getDuration() * 1E+6 << " us transmission duration"
             << " from " << radio << " on " << (IRadioMedium *)this << "." << endl;
    if (neighborCache && rangeFilter != RANGE_FILTER_ANYWHERE) {
        double range;
        if (rangeFilter == RANGE_FILTER_COMMUNICATION_RANGE)
            range = mediumLimitCache->getMaxCommunicationRange(radio).get();
        else if (rangeFilter == RANGE_FILTER_INTERFERENCE_RANGE)
            range = mediumLimitCache->getMaxInterferenceRange(radio).get();
        else
            throw cRuntimeError("Unknown range filter %d", rangeFilter);
        if (std::isnan(range)) {
            EV_WARN << "We can't use the NeighborCache for radio " << radio->getId() << ": range is NaN" << endl;
            sendToAllRadios(radio, transmittedSignal);
        }
        else
            neighborCache->sendToNeighbors(radio, transmittedSignal, range);
    }
    else
        sendToAllRadios(radio, transmittedSignal);

}

void RadioMedium::sendToRadio(IRadio *transmitter, const IRadio *receiver, const ISignal *transmittedSignal)
{
    const ITransmission *transmission = transmittedSignal->getTransmission();
    if (receiver != transmitter && receiver->getReceiver() != nullptr && isPotentialReceiver(receiver, transmission)) {
        auto transmitterRadio = const_cast<Radio *>(check_and_cast<const Radio *>(transmitter));
        cMethodCallContextSwitcher contextSwitcher(transmitterRadio);
        contextSwitcher.methodCall("sendToRadio");
        const IArrival *arrival = getArrival(receiver, transmission);
        simtime_t propagationTime = arrival->getStartPropagationTime();
        EV_DEBUG << "Sending " << transmittedSignal
                 << " from " << transmitter << " at " << transmission->getStartPosition()
                 << " to " << receiver << " at " << arrival->getStartPosition()
                 << " in " << propagationTime * 1E+6 << " us propagation time." << endl;
        auto receivedSignal = static_cast<Signal *>(createReceiverSignal(transmission));
        cGate *gate = receiver->getRadioGate()->getPathStartGate();
        transmitterRadio->sendDirect(receivedSignal, propagationTime, transmission->getDuration(), gate);
        communicationCache->setCachedSignal(receiver, transmission, receivedSignal);
        signalSendCount++;
    }
}

ISignal *RadioMedium::transmitPacket(const IRadio *radio, Packet *packet)
{
    Enter_Method("transmitPacket");
    auto signal = createTransmitterSignal(radio, packet);
    auto transmission = signal->getTransmission();
    addTransmission(radio, transmission);
    if (recordCommunicationLog)
        communicationLog.writeTransmission(radio, signal);
    sendToAffectedRadios(const_cast<IRadio *>(radio), signal);
    communicationCache->setCachedSignal(transmission, signal);
    return signal;
}

Packet *RadioMedium::receivePacket(const IRadio *radio, ISignal *signal)
{
    Enter_Method("receivePacket");
    const ITransmission *transmission = signal->getTransmission();
    const IListening *listening = communicationCache->getCachedListening(radio, transmission);
    if (recordCommunicationLog)
        communicationLog.writeReception(radio, signal);
    const IReceptionResult *result = getReceptionResult(radio, listening, transmission);
    communicationCache->removeCachedReceptionResult(radio, transmission);
    Packet *packet = result->getPacket()->dup();
    delete result;
    return packet;
}

const ITransmission *RadioMedium::getTransmission(int id) const
{
    return communicationCache->getTransmission(id);
}

const IListeningDecision *RadioMedium::listenOnMedium(const IRadio *radio, const IListening *listening) const
{
    Enter_Method("listenOnMedium");
    const IListeningDecision *decision = computeListeningDecision(radio, listening);
    EV_DEBUG << "Listening results in: " << decision << " with " << listening << " on medium by " << radio << endl;
    return decision;
}

bool RadioMedium::isPotentialReceiver(const IRadio *radio, const ITransmission *transmission) const
{
    const Radio *receiverRadio = dynamic_cast<const Radio *>(radio);
    if (radioModeFilter && receiverRadio != nullptr && receiverRadio->getRadioMode() != IRadio::RADIO_MODE_RECEIVER && receiverRadio->getRadioMode() != IRadio::RADIO_MODE_TRANSCEIVER)
        return false;
    else if (listeningFilter && radio->getReceiver() != nullptr && !radio->getReceiver()->computeIsReceptionPossible(getListening(radio, transmission), transmission))
        return false;
    // TODO: where is the tag?
    else if (macAddressFilter && !matchesMacAddressFilter(radio, transmission->getPacket()))
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
    const IInterference *interference = computeInterference(receiver, listening, transmission);
    bool isReceptionPossible = receiver->getReceiver()->computeIsReceptionAttempted(listening, reception, part, interference);
    delete interference;
    return isReceptionPossible;
}

bool RadioMedium::isReceptionAttempted(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part) const
{
    const IReception *reception = getReception(receiver, transmission);
    const IListening *listening = getListening(receiver, transmission);
    // TODO: why compute?
    const IInterference *interference = computeInterference(receiver, listening, transmission);
    bool isReceptionAttempted = receiver->getReceiver()->computeIsReceptionAttempted(listening, reception, part, interference);
    delete interference;
    return isReceptionAttempted;
}

bool RadioMedium::isReceptionSuccessful(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part) const
{
    const IReception *reception = getReception(receiver, transmission);
    const IListening *listening = getListening(receiver, transmission);
    // TODO: why compute?
    const IInterference *interference = computeInterference(receiver, listening, transmission);
    const ISnir *snir = getSNIR(receiver, transmission);
    bool isReceptionSuccessful = receiver->getReceiver()->computeIsReceptionSuccessful(listening, reception, part, interference, snir);
    delete interference;
    return isReceptionSuccessful;
}

void RadioMedium::sendToAllRadios(IRadio *transmitter, const ISignal *signal)
{
    communicationCache->mapRadios([&] (const IRadio *radio) {
        sendToRadio(transmitter, radio, signal);
    });
}

void RadioMedium::pickUpSignals(IRadio *receiverRadio)
{
    communicationCache->mapTransmissions([&] (const ITransmission *transmission) {
        auto transmitterRadio = dynamic_cast<const Radio *>(getRadio(transmission->getTransmitterId()));
        if (!transmitterRadio)
            return;
        if (communicationCache->getCachedSignal(receiverRadio, transmission) == nullptr &&
            receiverRadio != transmitterRadio && isPotentialReceiver(receiverRadio, transmission))
        {
            const IArrival *arrival = getArrival(receiverRadio, transmission);
            if (arrival->getEndTime() >= simTime()) {
                cMethodCallContextSwitcher contextSwitcher(transmitterRadio);
                contextSwitcher.methodCall("sendToRadio");
                const Packet *packet = transmission->getPacket();
                EV_DEBUG << "Picking up " << packet << " originally sent "
                         << " from " << (IRadio *)transmitterRadio << " at " << transmission->getStartPosition()
                         << " to " << (IRadio *)receiverRadio << " at " << arrival->getStartPosition()
                         << " in " << arrival->getStartPropagationTime() * 1E+6 << " us propagation time." << endl;
                auto signal = static_cast<Signal *>(createReceiverSignal(transmission));
                simtime_t delay = arrival->getStartTime() - simTime();
                simtime_t duration = delay > 0 ? signal->getDuration() : signal->getDuration() + delay;
                cGate *gate = receiverRadio->getRadioGate()->getPathStartGate();
                const_cast<Radio *>(transmitterRadio)->sendDirect(signal, delay > 0 ? delay : 0, duration, gate);
                communicationCache->setCachedSignal(receiverRadio, transmission, signal);
                signalSendCount++;
            }
        }
    });
}

void RadioMedium::receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details)
{
    if (signal == IRadio::radioModeChangedSignal) {
        Enter_Method("radioModeChanged");
        auto radio = check_and_cast<Radio *>(source);
        pickUpSignals(radio);
    }
    else if (signal == IRadio::listeningChangedSignal) {
        Enter_Method("listeningChanged");
        auto radio = check_and_cast<Radio *>(source);
        communicationCache->mapTransmissions([&] (const ITransmission *transmission) {
            const IArrival *arrival = getArrival(radio, transmission);
            const IListening *listening = radio->getReceiver()->createListening(radio, arrival->getStartTime(), arrival->getEndTime(), arrival->getStartPosition(), arrival->getEndPosition());
            delete communicationCache->getCachedListening(radio, transmission);
            communicationCache->setCachedListening(radio, transmission, listening);
        });
        pickUpSignals(radio);
    }
    else
        throw cRuntimeError("Unknown signal");
}

void RadioMedium::receiveSignal(cComponent *source, simsignal_t signal, cObject *value, cObject *details)
{
    if (signal == interfaceConfigChangedSignal) {
        Enter_Method("interfaceConfigChanged");
        auto interfaceChange = check_and_cast<InterfaceEntryChangeDetails *>(value);
        if (interfaceChange->getFieldId() == InterfaceEntry::F_MACADDRESS) {
            auto radio = check_and_cast<Radio *>(interfaceChange->getInterfaceEntry()->getSubmodule("radio"));
            pickUpSignals(radio);
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace physicallayer
} // namespace inet

