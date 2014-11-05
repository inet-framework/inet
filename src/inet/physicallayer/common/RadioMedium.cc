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

#include "inet/physicallayer/common/Radio.h"
#include "inet/physicallayer/common/RadioMedium.h"
#include "inet/physicallayer/common/Interference.h"
#include "inet/physicallayer/contract/RadioControlInfo_m.h"
#include "inet/linklayer/contract/IMACFrame.h"
#include "inet/common/NotifierConsts.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/environment/PhysicalEnvironment.h"

namespace inet {

namespace physicallayer {

Define_Module(RadioMedium);

RadioMedium::ReceptionCacheEntry::ReceptionCacheEntry() :
    frame(NULL),
    arrival(NULL),
    listening(NULL),
    reception(NULL),
    interference(NULL),
    noise(NULL),
    snir(NULL),
    decision(NULL)
{
}

RadioMedium::TransmissionCacheEntry::TransmissionCacheEntry() :
    frame(NULL),
    figure(NULL),
    receptionCacheEntries(NULL)
{
}

RadioMedium::RadioMedium() :
    propagation(NULL),
    pathLoss(NULL),
    analogModel(NULL),
    backgroundNoise(NULL),
    maxSpeed(mps(sNaN)),
    maxTransmissionPower(W(sNaN)),
    minInterferencePower(W(sNaN)),
    minReceptionPower(W(sNaN)),
    maxAntennaGain(sNaN),
    minInterferenceTime(sNaN),
    maxTransmissionDuration(sNaN),
    maxCommunicationRange(m(sNaN)),
    maxInterferenceRange(m(sNaN)),
    rangeFilter(RANGE_FILTER_ANYWHERE),
    radioModeFilter(false),
    listeningFilter(false),
    macAddressFilter(false),
    recordCommunicationLog(false),
    displayCommunication(false),
    drawCommunication2D(false),
    leaveCommunicationTrail(false),
    updateCanvasInterval(sNaN),
    updateCanvasTimer(NULL),
    removeNonInterferingTransmissionsTimer(NULL),
    baseRadioId(0),
    baseTransmissionId(0),
    neighborCache(NULL),
    communicationLayer(NULL),
    communicationTrail(NULL),
    transmissionCount(0),
    sendCount(0),
    receptionComputationCount(0),
    interferenceComputationCount(0),
    receptionDecisionComputationCount(0),
    listeningDecisionComputationCount(0),
    cacheReceptionGetCount(0),
    cacheReceptionHitCount(0),
    cacheInterferenceGetCount(0),
    cacheInterferenceHitCount(0),
    cacheDecisionGetCount(0),
    cacheDecisionHitCount(0)
{
}

RadioMedium::~RadioMedium()
{
    delete backgroundNoise;
    for (std::vector<const ITransmission *>::const_iterator it = transmissions.begin(); it != transmissions.end(); it++)
        delete *it;
    cancelAndDelete(updateCanvasTimer);
    cancelAndDelete(removeNonInterferingTransmissionsTimer);
    for (std::vector<TransmissionCacheEntry>::const_iterator it = cache.begin(); it != cache.end(); it++) {
        const TransmissionCacheEntry& transmissionCacheEntry = *it;
        delete transmissionCacheEntry.frame;
        const std::vector<ReceptionCacheEntry> *receptionCacheEntries = transmissionCacheEntry.receptionCacheEntries;
        if (receptionCacheEntries) {
            for (std::vector<ReceptionCacheEntry>::const_iterator jt = receptionCacheEntries->begin(); jt != receptionCacheEntries->end(); jt++) {
                const ReceptionCacheEntry& cacheEntry = *jt;
                delete cacheEntry.arrival;
                delete cacheEntry.listening;
                delete cacheEntry.reception;
                delete cacheEntry.interference;
                delete cacheEntry.noise;
                delete cacheEntry.snir;
                delete cacheEntry.decision;
            }
            delete receptionCacheEntries;
        }
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
        neighborCache = dynamic_cast<INeighborCache *>(getSubmodule("neighborCache"));
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
        updateCanvasTimer = new cMessage("updateCanvas");
        removeNonInterferingTransmissionsTimer = new cMessage("removeNonInterferingTransmissions");
        // initialize logging
        recordCommunicationLog = par("recordCommunicationLog");
        if (recordCommunicationLog)
            communicationLog.open(ev.getConfig()->substituteVariables("${resultdir}/${configname}-${runnumber}.tlog"));
        // initialize graphics
        displayCommunication = par("displayCommunication");
        drawCommunication2D = par("drawCommunication2D");
        cCanvas *canvas = getParentModule()->getCanvas();
        if (displayCommunication) {
            communicationLayer = new cGroupFigure("communication");
            canvas->addFigure(communicationLayer, canvas->findFigure("submodules"));
        }
        leaveCommunicationTrail = par("leaveCommunicationTrail");
        if (leaveCommunicationTrail) {
            communicationTrail = new TrailFigure(100, "communication trail");
            canvas->addFigure(communicationTrail, canvas->findFigure("submodules"));
        }
        updateCanvasInterval = par("updateCanvasInterval");
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER)
        updateLimits();
    else if (stage == INITSTAGE_LAST) {
        EV_DEBUG << "Initialized ";
        printToStream(EVSTREAM);
        EV_DEBUG << endl;
    }
}

void RadioMedium::finish()
{
    double receptionCacheHitPercentage = 100 * (double)cacheReceptionHitCount / (double)cacheReceptionGetCount;
    double interferenceCacheHitPercentage = 100 * (double)cacheInterferenceHitCount / (double)cacheInterferenceGetCount;
    double decisionCacheHitPercentage = 100 * (double)cacheDecisionHitCount / (double)cacheDecisionGetCount;
    EV_INFO << "Transmission count = " << transmissionCount << endl;
    EV_INFO << "Radio frame send count = " << sendCount << endl;
    EV_INFO << "Reception computation count = " << receptionComputationCount << endl;
    EV_INFO << "Interference computation count = " << interferenceComputationCount << endl;
    EV_INFO << "Reception decision computation count = " << receptionDecisionComputationCount << endl;
    EV_INFO << "Listening decision computation count = " << listeningDecisionComputationCount << endl;
    EV_INFO << "Reception cache hit = " << receptionCacheHitPercentage << " %" << endl;
    EV_INFO << "Interference cache hit = " << interferenceCacheHitPercentage << " %" << endl;
    EV_INFO << "Reception decision cache hit = " << decisionCacheHitPercentage << " %" << endl;
    recordScalar("transmission count", transmissionCount);
    recordScalar("radio frame send count", sendCount);
    recordScalar("reception computation count", receptionComputationCount);
    recordScalar("interference computation count", interferenceComputationCount);
    recordScalar("reception decision computation count", receptionDecisionComputationCount);
    recordScalar("listening decision computation count", listeningDecisionComputationCount);
    recordScalar("reception cache hit", receptionCacheHitPercentage, "%");
    recordScalar("interference cache hit", interferenceCacheHitPercentage, "%");
    recordScalar("reception decision cache hit", decisionCacheHitPercentage, "%");
}

void RadioMedium::printToStream(std::ostream& stream) const
{
    stream << "RadioMedium, "
           << "maxTransmissionPower = " << maxTransmissionPower << ", "
           << "minInterferencePower = " << minInterferencePower << ", "
           << "minReceptionPower = " << minReceptionPower << ", "
           << "maxAntennaGain = " << maxAntennaGain << ", "
           << "minInterferenceTime = " << minInterferenceTime << ", "
           << "maxTransmissionDuration = " << maxTransmissionDuration << ", "
           << "maxCommunicationRange = " << maxCommunicationRange << ", "
           << "maxInterferenceRange = " << maxInterferenceRange << ", "
           << "propagation = { " << propagation << " }, "
           << "analogModel = { " << analogModel << " }, "
           << "pathLoss = { " << pathLoss << " }, ";
    if (obstacleLoss)
        stream << "obstacleLoss = { " << obstacleLoss << " }, ";
    else
        stream << "obstacleLoss = NULL, ";
    if (backgroundNoise)
        stream << "backgroundNoise = { " << backgroundNoise << " }";
    else
        stream << "backgroundNoise = NULL";
}

void RadioMedium::handleMessage(cMessage *message)
{
    if (message == removeNonInterferingTransmissionsTimer) {
        removeNonInterferingTransmissions();
        if (displayCommunication)
            updateCanvas();
    }
    else if (message == updateCanvasTimer) {
        updateCanvas();
        scheduleUpdateCanvasTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

RadioMedium::TransmissionCacheEntry *RadioMedium::getTransmissionCacheEntry(const ITransmission *transmission) const
{
    int transmissionIndex = transmission->getId() - baseTransmissionId;
    if (transmissionIndex < 0)
        return NULL;
    else {
        if (transmissionIndex >= cache.size())
            cache.resize(transmissionIndex + 1);
        TransmissionCacheEntry& transmissionCacheEntry = cache[transmissionIndex];
        if (!transmissionCacheEntry.receptionCacheEntries)
            transmissionCacheEntry.receptionCacheEntries = new std::vector<ReceptionCacheEntry>(radios.size());
        return &transmissionCacheEntry;
    }
}

RadioMedium::ReceptionCacheEntry *RadioMedium::getReceptionCacheEntry(const IRadio *radio, const ITransmission *transmission) const
{
    TransmissionCacheEntry *transmissionCacheEntry = getTransmissionCacheEntry(transmission);
    if (!transmissionCacheEntry)
        return NULL;
    else {
        std::vector<ReceptionCacheEntry> *receptionCacheEntries = transmissionCacheEntry->receptionCacheEntries;
        int radioIndex = radio->getId() - baseRadioId;
        if (radioIndex < 0)
            return NULL;
        else {
            if (radioIndex >= receptionCacheEntries->size())
                receptionCacheEntries->resize(radioIndex + 1);
            return &(*receptionCacheEntries)[radioIndex];
        }
    }
}

const IArrival *RadioMedium::getCachedArrival(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->arrival : NULL;
}

void RadioMedium::setCachedArrival(const IRadio *radio, const ITransmission *transmission, const IArrival *arrival) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->arrival = arrival;
}

void RadioMedium::removeCachedArrival(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->arrival = NULL;
}

const IListening *RadioMedium::getCachedListening(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->listening : NULL;
}

void RadioMedium::setCachedListening(const IRadio *radio, const ITransmission *transmission, const IListening *listening) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->listening = listening;
}

void RadioMedium::removeCachedListening(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->listening = NULL;
}

const IReception *RadioMedium::getCachedReception(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->reception : NULL;
}

void RadioMedium::setCachedReception(const IRadio *radio, const ITransmission *transmission, const IReception *reception) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->reception = reception;
}

void RadioMedium::removeCachedReception(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->reception = NULL;
}

const IInterference *RadioMedium::getCachedInterference(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->interference : NULL;
}

void RadioMedium::setCachedInterference(const IRadio *radio, const ITransmission *transmission, const IInterference *interference) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->interference = interference;
}

void RadioMedium::removeCachedInterference(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->interference = NULL;
}

const INoise *RadioMedium::getCachedNoise(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->noise : NULL;
}

void RadioMedium::setCachedNoise(const IRadio *radio, const ITransmission *transmission, const INoise *noise) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->noise = noise;
}

void RadioMedium::removeCachedNoise(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->noise = NULL;
}

const ISNIR *RadioMedium::getCachedSNIR(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->snir : NULL;
}

void RadioMedium::setCachedSNIR(const IRadio *radio, const ITransmission *transmission, const ISNIR *snir) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->snir = snir;
}

void RadioMedium::removeCachedSNIR(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->snir = NULL;
}

const IReceptionDecision *RadioMedium::getCachedDecision(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->decision : NULL;
}

void RadioMedium::setCachedDecision(const IRadio *radio, const ITransmission *transmission, const IReceptionDecision *decision) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->decision = decision;
}

void RadioMedium::removeCachedDecision(const IRadio *radio, const ITransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->decision = NULL;
}

void RadioMedium::invalidateCachedDecisions(const ITransmission *transmission)
{
    for (std::vector<TransmissionCacheEntry>::iterator it = cache.begin(); it != cache.end(); it++) {
        const TransmissionCacheEntry& transmissionCacheEntry = *it;
        std::vector<ReceptionCacheEntry> *receptionCacheEntries = transmissionCacheEntry.receptionCacheEntries;
        if (receptionCacheEntries) {
            for (std::vector<ReceptionCacheEntry>::iterator jt = receptionCacheEntries->begin(); jt != receptionCacheEntries->end(); jt++) {
                ReceptionCacheEntry& cacheEntry = *jt;
                const IReceptionDecision *decision = cacheEntry.decision;
                if (decision) {
                    const IReception *reception = decision->getReception();
                    if (isInterferingTransmission(transmission, reception))
                        invalidateCachedDecision(decision);
                }
            }
        }
    }
}

void RadioMedium::invalidateCachedDecision(const IReceptionDecision *decision)
{
    const IReception *reception = decision->getReception();
    const IRadio *radio = reception->getReceiver();
    const ITransmission *transmission = reception->getTransmission();
    ReceptionCacheEntry *receptionCacheEntry = getReceptionCacheEntry(radio, transmission);
    if (receptionCacheEntry)
        receptionCacheEntry->decision = NULL;
}

template<typename T> inline T minIgnoreNaN(T a, T b)
{
    if (isNaN(a.get()))
        return b;
    else if (isNaN(b.get()))
        return a;
    else if (a < b)
        return a;
    else
        return b;
}

template<typename T> inline T maxIgnoreNaN(T a, T b)
{
    if (isNaN(a.get()))
        return b;
    else if (isNaN(b.get()))
        return a;
    else if (a > b)
        return a;
    else
        return b;
}

inline double maxIgnoreNaN(double a, double b)
{
    if (isNaN(a))
        return b;
    else if (isNaN(b))
        return a;
    else if (a > b)
        return a;
    else
        return b;
}

mps RadioMedium::computeMaxSpeed() const
{
    mps maxSpeed = mps(par("maxSpeed"));
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
        maxSpeed = maxIgnoreNaN(maxSpeed, mps((*it)->getAntenna()->getMobility()->getMaxSpeed()));
    return maxSpeed;
}

W RadioMedium::computeMaxTransmissionPower() const
{
    W maxTransmissionPower = W(par("maxTransmissionPower"));
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
        maxTransmissionPower = maxIgnoreNaN(maxTransmissionPower, (*it)->getTransmitter()->getMaxPower());
    return maxTransmissionPower;
}

W RadioMedium::computeMinInterferencePower() const
{
    W minInterferencePower = mW(math::dBm2mW(par("minInterferencePower")));
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
        minInterferencePower = minIgnoreNaN(minInterferencePower, (*it)->getReceiver()->getMinInterferencePower());
    return minInterferencePower;
}

W RadioMedium::computeMinReceptionPower() const
{
    W minReceptionPower = mW(math::dBm2mW(par("minReceptionPower")));
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
        minReceptionPower = minIgnoreNaN(minReceptionPower, (*it)->getReceiver()->getMinReceptionPower());
    return minReceptionPower;
}

double RadioMedium::computeMaxAntennaGain() const
{
    double maxAntennaGain = math::dB2fraction(par("maxAntennaGain"));
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
        maxAntennaGain = maxIgnoreNaN(maxAntennaGain, (*it)->getAntenna()->getMaxGain());
    return maxAntennaGain;
}

m RadioMedium::computeMaxRange(W maxTransmissionPower, W minReceptionPower) const
{
    Hz carrierFrequency = Hz(par("carrierFrequency"));
    double loss = unit(minReceptionPower / maxTransmissionPower).get() / maxAntennaGain / maxAntennaGain;
    return pathLoss->computeRange(propagation->getPropagationSpeed(), carrierFrequency, loss);
}

m RadioMedium::computeMaxCommunicationRange() const
{
    return maxIgnoreNaN(m(par("maxCommunicationRange")), computeMaxRange(maxTransmissionPower, minReceptionPower));
}

m RadioMedium::computeMaxInterferenceRange() const
{
    return maxIgnoreNaN(m(par("maxInterferenceRange")), computeMaxRange(maxTransmissionPower, minInterferencePower));
}

const simtime_t RadioMedium::computeMinInterferenceTime() const
{
    return par("minInterferenceTime").doubleValue();
}

const simtime_t RadioMedium::computeMaxTransmissionDuration() const
{
    return par("maxTransmissionDuration").doubleValue();
}

void RadioMedium::updateLimits()
{
    maxSpeed = computeMaxSpeed();
    maxTransmissionPower = computeMaxTransmissionPower();
    minInterferencePower = computeMinInterferencePower();
    minReceptionPower = computeMinReceptionPower();
    maxAntennaGain = computeMaxAntennaGain();
    minInterferenceTime = computeMinInterferenceTime();
    maxTransmissionDuration = computeMaxTransmissionDuration();
    maxCommunicationRange = computeMaxCommunicationRange();
    maxInterferenceRange = computeMaxInterferenceRange();
    constraintAreaMin = computeConstraintAreaMin();
    constraintAreaMax = computeConstreaintAreaMax();
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
    return isNaN(maxCommunicationRange.get()) ||
           (transmission->getStartPosition().distance(startPosition) < maxCommunicationRange.get() &&
            transmission->getEndPosition().distance(endPosition) < maxCommunicationRange.get());
}

bool RadioMedium::isInInterferenceRange(const ITransmission *transmission, const Coord startPosition, const Coord endPosition) const
{
    return isNaN(maxInterferenceRange.get()) ||
           (transmission->getStartPosition().distance(startPosition) < maxInterferenceRange.get() &&
            transmission->getEndPosition().distance(endPosition) < maxInterferenceRange.get());
}

bool RadioMedium::isInterferingTransmission(const ITransmission *transmission, const IListening *listening) const
{
    const IRadio *transmitter = transmission->getTransmitter();
    const IRadio *receiver = listening->getReceiver();
    const IArrival *arrival = getArrival(receiver, transmission);
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
    return transmitter != receiver &&
           arrival->getEndTime() > reception->getStartTime() + minInterferenceTime &&
           arrival->getStartTime() < reception->getEndTime() - minInterferenceTime &&
           isInInterferenceRange(transmission, reception->getStartPosition(), reception->getEndPosition());
}

void RadioMedium::removeNonInterferingTransmissions()
{
    const simtime_t now = simTime();
    size_t transmissionIndex = 0;
    while (transmissionIndex < cache.size() && cache[transmissionIndex].interferenceEndTime <= now)
        transmissionIndex++;
    EV_DEBUG << "Removing " << transmissionIndex << " non interfering transmissions\n";
    baseTransmissionId += transmissionIndex;
    for (std::vector<const ITransmission *>::const_iterator it = transmissions.begin(); it != transmissions.begin() + transmissionIndex; it++)
        delete *it;
    transmissions.erase(transmissions.begin(), transmissions.begin() + transmissionIndex);
    for (std::vector<TransmissionCacheEntry>::const_iterator it = cache.begin(); it != cache.begin() + transmissionIndex; it++) {
        const TransmissionCacheEntry& transmissionCacheEntry = *it;
        delete transmissionCacheEntry.frame;
        const std::vector<ReceptionCacheEntry> *receptionCacheEntries = transmissionCacheEntry.receptionCacheEntries;
        if (receptionCacheEntries) {
            for (std::vector<ReceptionCacheEntry>::const_iterator jt = receptionCacheEntries->begin(); jt != receptionCacheEntries->end(); jt++) {
                const ReceptionCacheEntry& cacheEntry = *jt;
                delete cacheEntry.arrival;
                delete cacheEntry.listening;
                delete cacheEntry.reception;
                delete cacheEntry.interference;
                delete cacheEntry.noise;
                delete cacheEntry.snir;
                delete cacheEntry.decision;
            }
            delete receptionCacheEntries;
            if (displayCommunication && transmissionCacheEntry.figure)
                delete communicationLayer->removeFigure(transmissionCacheEntry.figure);
        }
    }
    cache.erase(cache.begin(), cache.begin() + transmissionIndex);
    if (cache.size() > 0)
        scheduleAt(cache[0].interferenceEndTime, removeNonInterferingTransmissionsTimer);
}

const std::vector<const IReception *> *RadioMedium::computeInterferingReceptions(const IListening *listening, const std::vector<const ITransmission *> *transmissions) const
{
    const IRadio *radio = listening->getReceiver();
    std::vector<const IReception *> *interferingReceptions = new std::vector<const IReception *>();
    for (std::vector<const ITransmission *>::const_iterator it = transmissions->begin(); it != transmissions->end(); it++) {
        const ITransmission *interferingTransmission = *it;
        if (interferingTransmission->getTransmitter() != radio && isInterferingTransmission(interferingTransmission, listening))
            interferingReceptions->push_back(getReception(radio, interferingTransmission));
    }
    return interferingReceptions;
}

const std::vector<const IReception *> *RadioMedium::computeInterferingReceptions(const IReception *reception, const std::vector<const ITransmission *> *transmissions) const
{
    const IRadio *radio = reception->getReceiver();
    const ITransmission *transmission = reception->getTransmission();
    std::vector<const IReception *> *interferingReceptions = new std::vector<const IReception *>();
    for (std::vector<const ITransmission *>::const_iterator it = transmissions->begin(); it != transmissions->end(); it++) {
        const ITransmission *interferingTransmission = *it;
        if (transmission != interferingTransmission && interferingTransmission->getTransmitter() != radio && isInterferingTransmission(interferingTransmission, reception))
            interferingReceptions->push_back(getReception(radio, interferingTransmission));
    }
    return interferingReceptions;
}

const IReception *RadioMedium::computeReception(const IRadio *radio, const ITransmission *transmission) const
{
    receptionComputationCount++;
    return analogModel->computeReception(radio, transmission);
}

const IInterference *RadioMedium::computeInterference(const IRadio *receiver, const IListening *listening, const std::vector<const ITransmission *> *transmissions) const
{
    interferenceComputationCount++;
    const INoise *noise = backgroundNoise ? backgroundNoise->computeNoise(listening) : NULL;
    const std::vector<const IReception *> *interferingReceptions = computeInterferingReceptions(listening, transmissions);
    return new Interference(noise, interferingReceptions);
}

const IInterference *RadioMedium::computeInterference(const IRadio *receiver, const IListening *listening, const ITransmission *transmission, const std::vector<const ITransmission *> *transmissions) const
{
    interferenceComputationCount++;
    const IReception *reception = getReception(receiver, transmission);
    const INoise *noise = backgroundNoise ? backgroundNoise->computeNoise(listening) : NULL;
    const std::vector<const IReception *> *interferingReceptions = computeInterferingReceptions(reception, transmissions);
    return new Interference(noise, interferingReceptions);
}

const IReceptionDecision *RadioMedium::computeReceptionDecision(const IRadio *radio, const IListening *listening, const ITransmission *transmission, const std::vector<const ITransmission *> *transmissions) const
{
    receptionDecisionComputationCount++;
    const IReception *reception = getReception(radio, transmission);
    const IInterference *interference = getInterference(radio, listening, transmission);
    const IReceptionDecision *decision = radio->getReceiver()->computeReceptionDecision(listening, reception, interference);
    return decision;
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
    return getCachedArrival(receiver, transmission);
}

const IReception *RadioMedium::getReception(const IRadio *receiver, const ITransmission *transmission) const
{
    cacheReceptionGetCount++;
    const IReception *reception = getCachedReception(receiver, transmission);
    if (reception)
        cacheReceptionHitCount++;
    else {
        reception = computeReception(receiver, transmission);
        setCachedReception(receiver, transmission, reception);
        EV_DEBUG << "Receiving " << transmission << " from medium by " << receiver << " arrives as " << reception << endl;
    }
    return reception;
}

const IInterference *RadioMedium::getInterference(const IRadio *receiver, const ITransmission *transmission) const
{
    return getInterference(receiver, getCachedListening(receiver, transmission), transmission);
}

const IInterference *RadioMedium::getInterference(const IRadio *receiver, const IListening *listening, const ITransmission *transmission) const
{
    cacheInterferenceGetCount++;
    const IInterference *interference = getCachedInterference(receiver, transmission);
    if (interference)
        cacheInterferenceHitCount++;
    else {
        interference = computeInterference(receiver, listening, transmission, const_cast<const std::vector<const ITransmission *> *>(&transmissions));
        setCachedInterference(receiver, transmission, interference);
    }
    return interference;
}

const INoise *RadioMedium::getNoise(const IRadio *receiver, const ITransmission *transmission) const
{
    const INoise *noise = getCachedNoise(receiver, transmission);
    if (!noise) {
        const IListening *listening = getCachedListening(receiver, transmission);
        const IInterference *interference = getInterference(receiver, transmission);
        noise = analogModel->computeNoise(listening, interference);
        setCachedNoise(receiver, transmission, noise);
    }
    return noise;
}

const ISNIR *RadioMedium::getSNIR(const IRadio *receiver, const ITransmission *transmission) const
{
    const ISNIR *snir = getCachedSNIR(receiver, transmission);
    if (!snir) {
        const IReception *reception = getReception(receiver, transmission);
        const INoise *noise = getNoise(receiver, transmission);
        snir = analogModel->computeSNIR(reception, noise);
        setCachedSNIR(receiver, transmission, snir);
    }
    return snir;
}

const IReceptionDecision *RadioMedium::getReceptionDecision(const IRadio *radio, const IListening *listening, const ITransmission *transmission) const
{
    cacheDecisionGetCount++;
    const IReceptionDecision *decision = getCachedDecision(radio, transmission);
    if (decision)
        cacheDecisionHitCount++;
    else {
        decision = computeReceptionDecision(radio, listening, transmission, const_cast<const std::vector<const ITransmission *> *>(&transmissions));
        setCachedDecision(radio, transmission, decision);
        EV_DEBUG << "Receiving " << transmission << " from medium by " << radio << " arrives as " << decision->getReception() << " and results in " << decision << endl;
    }
    return decision;
}

void RadioMedium::addRadio(const IRadio *radio)
{
    if (radios.size() == 0)
        baseRadioId = radio->getId();
    radios.push_back(radio);
    for (std::vector<const ITransmission *>::const_iterator it = transmissions.begin(); it != transmissions.end(); it++) {
        const ITransmission *transmission = *it;
        const IArrival *arrival = propagation->computeArrival(transmission, radio->getAntenna()->getMobility());
        const IListening *listening = radio->getReceiver()->createListening(radio, arrival->getStartTime(), arrival->getEndTime(), arrival->getStartPosition(), arrival->getEndPosition());
        setCachedArrival(radio, transmission, arrival);
        setCachedListening(radio, transmission, listening);
    }
    updateLimits();
    cModule *radioModule = const_cast<cModule *>(check_and_cast<const cModule *>(radio));
    if (radioModeFilter)
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
    if (listeningFilter)
        radioModule->subscribe(IRadio::listeningChangedSignal, this);
    if (macAddressFilter)
        getContainingNode(radioModule)->subscribe(NF_INTERFACE_CONFIG_CHANGED, this);
    if (neighborCache)
        neighborCache->addRadio(radio);
}

void RadioMedium::removeRadio(const IRadio *radio)
{
    radios.erase(std::remove(radios.begin(), radios.end(), radio));
    if (initialized())
        updateLimits();
    if (neighborCache)
        neighborCache->removeRadio(radio);
}

void RadioMedium::addTransmission(const IRadio *transmitterRadio, const ITransmission *transmission)
{
    if (transmissions.size() == 0)
        baseTransmissionId = transmission->getId();
    transmissionCount++;
    transmissions.push_back(transmission);
    simtime_t maxArrivalEndTime = simTime();
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++) {
        const IRadio *receiverRadio = *it;
        if (receiverRadio != transmitterRadio) {
            const IArrival *arrival = propagation->computeArrival(transmission, receiverRadio->getAntenna()->getMobility());
            const IListening *listening = receiverRadio->getReceiver()->createListening(receiverRadio, arrival->getStartTime(), arrival->getEndTime(), arrival->getStartPosition(), arrival->getEndPosition());
            const simtime_t arrivalEndTime = arrival->getEndTime();
            if (arrivalEndTime > maxArrivalEndTime)
                maxArrivalEndTime = arrivalEndTime;
            setCachedArrival(receiverRadio, transmission, arrival);
            setCachedListening(receiverRadio, transmission, listening);
        }
    }
    getTransmissionCacheEntry(transmission)->interferenceEndTime = maxArrivalEndTime + maxTransmissionDuration;
    if (!removeNonInterferingTransmissionsTimer->isScheduled()) {
        Enter_Method_Silent();
        scheduleAt(cache[0].interferenceEndTime, removeNonInterferingTransmissionsTimer);
    }
    if (updateCanvasInterval > 0 && !updateCanvasTimer->isScheduled())
        scheduleUpdateCanvasTimer();
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
            range = getMaxCommunicationRange(radio).get();
        else if (rangeFilter == RANGE_FILTER_INTERFERENCE_RANGE)
            range = getMaxInterferenceRange(radio).get();
        else
            throw cRuntimeError("Unknown range filter %d", rangeFilter);
        if (isNaN(range))
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
    const RadioFrame *radioFrame = check_and_cast<const RadioFrame *>(frame);
    const ITransmission *transmission = frame->getTransmission();
    ReceptionCacheEntry *receptionCacheEntry = getReceptionCacheEntry(receiverRadio, transmission);
    if (receiverRadio != transmitterRadio && isPotentialReceiver(receiverRadio, transmission)) {
        const IArrival *arrival = getArrival(receiverRadio, transmission);
        simtime_t propagationTime = arrival->getStartPropagationTime();
        EV_DEBUG << "Sending " << frame
                 << " from " << (IRadio *)transmitterRadio << " at " << transmission->getStartPosition()
                 << " to " << (IRadio *)receiverRadio << " at " << arrival->getStartPosition()
                 << " in " << propagationTime * 1E+6 << " us propagation time." << endl;
        RadioFrame *frameCopy = radioFrame->dup();
        cGate *gate = receiverRadio->getRadioGate()->getPathStartGate();
        const_cast<Radio *>(transmitterRadio)->sendDirect(frameCopy, propagationTime, radioFrame->getDuration(), gate);
        receptionCacheEntry->frame = frameCopy;
        sendCount++;
    }
}

IRadioFrame *RadioMedium::transmitPacket(const IRadio *radio, cPacket *macFrame)
{
    const ITransmission *transmission = radio->getTransmitter()->createTransmission(radio, macFrame, simTime()); // TODO: memory leak
    addTransmission(radio, transmission);
    RadioFrame *radioFrame = new RadioFrame(transmission);
    radioFrame->setName(macFrame->getName());
    radioFrame->setDuration(transmission->getEndTime() - transmission->getStartTime());
    radioFrame->encapsulate(macFrame);
    if (recordCommunicationLog) {
        const Radio *transmitterRadio = check_and_cast<const Radio *>(radio);
        communicationLog << "T " << transmitterRadio->getFullPath() << " " << transmitterRadio->getId() << " "
                         << "M " << radioFrame->getName() << " " << transmission->getId() << " "
                         << "S " << transmission->getStartTime() << " " << transmission->getStartPosition() << " -> "
                         << "E " << transmission->getEndTime() << " " << transmission->getEndPosition() << endl;
    }
    sendToAffectedRadios(const_cast<IRadio *>(radio), radioFrame);
    cMethodCallContextSwitcher contextSwitcher(this);
    TransmissionCacheEntry *transmissionCacheEntry = getTransmissionCacheEntry(transmission);
    transmissionCacheEntry->frame = radioFrame->dup();
//#if OMNETPP_CANVAS_VERSION >= 0x20140908
    if (displayCommunication) {
        cFigure::Point position = PhysicalEnvironment::computeCanvasPoint(transmission->getStartPosition());
        cGroupFigure *groupFigure = new cGroupFigure();
#if OMNETPP_CANVAS_VERSION >= 0x20140908
        cFigure::Color color = cFigure::GOOD_DARK_COLORS[transmission->getId() % (sizeof(cFigure::GOOD_DARK_COLORS) / sizeof(cFigure::Color))];
        cRingFigure *communicationFigure = new cRingFigure();
#else
        cFigure::Color color(64 + rand() % 64, 64 + rand() % 64, 64 + rand() % 64);
        cOvalFigure *communicationFigure = new cOvalFigure();
#endif
        communicationFigure->setTags("ongoing_transmission");
        communicationFigure->setBounds(cFigure::Rectangle(position.x, position.y, 0, 0));
        communicationFigure->setFillColor(color);
        communicationFigure->setLineWidth(1);
        communicationFigure->setLineColor(cFigure::BLACK);
        groupFigure->addFigure(communicationFigure);
#if OMNETPP_CANVAS_VERSION >= 0x20140908
        communicationFigure->setFilled(true);
        communicationFigure->setFillOpacity(0.5);
        communicationFigure->setLineOpacity(0.5);
        cLabelFigure *nameFigure = new cLabelFigure();
        nameFigure->setPosition(position);
#else
        cTextFigure *nameFigure = new cTextFigure();
        nameFigure->setLocation(position);
#endif
        nameFigure->setTags("ongoing_transmission packet_name label");
        nameFigure->setText(macFrame->getName());
        nameFigure->setColor(color);
        groupFigure->addFigure(nameFigure);
        communicationLayer->addFigure(groupFigure);
        transmissionCacheEntry->figure = groupFigure;
        updateCanvas();
    }
//#endif
    return radioFrame;
}

cPacket *RadioMedium::receivePacket(const IRadio *radio, IRadioFrame *radioFrame)
{
    const ITransmission *transmission = radioFrame->getTransmission();
    const IArrival *arrival = getArrival(radio, transmission);
    const IListening *listening = radio->getReceiver()->createListening(radio, arrival->getStartTime(), arrival->getEndTime(), arrival->getStartPosition(), arrival->getEndPosition());
    const IReceptionDecision *decision = getReceptionDecision(radio, listening, transmission);
    removeCachedDecision(radio, transmission);
    if (recordCommunicationLog) {
        const IReception *reception = decision->getReception();
        communicationLog << "R " << check_and_cast<const Radio *>(radio)->getFullPath() << " " << reception->getReceiver()->getId() << " "
                         << "M " << check_and_cast<const RadioFrame *>(radioFrame)->getName() << " " << transmission->getId() << " "
                         << "S " << reception->getStartTime() << " " << reception->getStartPosition() << " -> "
                         << "E " << reception->getEndTime() << " " << reception->getEndPosition() << " "
                         << "D " << decision->isReceptionPossible() << " " << decision->isReceptionAttempted() << " " << decision->isReceptionSuccessful() << endl;
    }
    cPacket *macFrame = check_and_cast<cPacket *>(radioFrame)->decapsulate();
    macFrame->setBitError(!decision->isReceptionSuccessful());
    macFrame->setControlInfo(const_cast<RadioReceptionIndication *>(decision->getIndication()));
    delete listening;
    if (leaveCommunicationTrail && decision->isReceptionSuccessful()) {
        cLineFigure *communicationFigure = new cLineFigure();
        communicationFigure->setTags("successful_reception recent_history");
        cFigure::Point start = PhysicalEnvironment::computeCanvasPoint(transmission->getStartPosition());
        cFigure::Point end = PhysicalEnvironment::computeCanvasPoint(decision->getReception()->getStartPosition());
        communicationFigure->setStart(start);
        communicationFigure->setEnd(end);
        communicationFigure->setLineColor(cFigure::BLUE);
        communicationFigure->setEndArrowHead(cFigure::ARROW_SIMPLE);
        communicationTrail->addFigure(communicationFigure);
    }
    if (displayCommunication)
        updateCanvas();
    delete decision;
    return macFrame;
}

const IListeningDecision *RadioMedium::listenOnMedium(const IRadio *radio, const IListening *listening) const
{
    const IListeningDecision *decision = computeListeningDecision(radio, listening, const_cast<const std::vector<const ITransmission *> *>(&transmissions));
    EV_DEBUG << "Listening " << listening << " on medium by " << radio << " results in " << decision << endl;
    return decision;
}

bool RadioMedium::isPotentialReceiver(const IRadio *radio, const ITransmission *transmission) const
{
    const Radio *receiverRadio = check_and_cast<const Radio *>(radio);
    if (radioModeFilter && receiverRadio->getRadioMode() != IRadio::RADIO_MODE_RECEIVER && receiverRadio->getRadioMode() != IRadio::RADIO_MODE_TRANSCEIVER)
        return false;
    else if (listeningFilter && !radio->getReceiver()->computeIsReceptionPossible(transmission))
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

bool RadioMedium::isReceptionAttempted(const IRadio *radio, const ITransmission *transmission) const
{
    const IReception *reception = getReception(radio, transmission);
    const IListening *listening = getCachedListening(radio, transmission);
    const IInterference *interference = computeInterference(radio, listening, transmission, const_cast<const std::vector<const ITransmission *> *>(&transmissions));
    bool isReceptionAttempted = radio->getReceiver()->computeIsReceptionAttempted(listening, reception, interference);
    delete interference;
    EV_DEBUG << "Receiving " << transmission << " from medium by " << radio << " arrives as " << reception << " and results in reception is " << (isReceptionAttempted ? "attempted" : "ignored") << endl;
    if (displayCommunication)
        const_cast<RadioMedium *>(this)->updateCanvas();
    return isReceptionAttempted;
}

void RadioMedium::sendToAllRadios(IRadio *transmitter, const IRadioFrame *frame)
{
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
        sendToRadio(transmitter, *it, frame);
}

void RadioMedium::receiveSignal(cComponent *source, simsignal_t signal, long value)
{
    if (signal == IRadio::radioModeChangedSignal || signal == IRadio::listeningChangedSignal || signal == NF_INTERFACE_CONFIG_CHANGED) {
        const Radio *receiverRadio = check_and_cast<const Radio *>(source);
        for (std::vector<const ITransmission *>::iterator it = transmissions.begin(); it != transmissions.end(); it++) {
            const ITransmission *transmission = *it;
            const Radio *transmitterRadio = check_and_cast<const Radio *>(transmission->getTransmitter());
            ReceptionCacheEntry *receptionCacheEntry = getReceptionCacheEntry(receiverRadio, transmission);
            if (receptionCacheEntry && signal == IRadio::listeningChangedSignal) {
                const IArrival *arrival = getArrival(receiverRadio, transmission);
                const IListening *listening = receiverRadio->getReceiver()->createListening(receiverRadio, arrival->getStartTime(), arrival->getEndTime(), arrival->getStartPosition(), arrival->getEndPosition());
                delete getCachedListening(receiverRadio, transmission);
                setCachedListening(receiverRadio, transmission, listening);
            }
            if (receptionCacheEntry && !receptionCacheEntry->frame && receiverRadio != transmitterRadio && isPotentialReceiver(receiverRadio, transmission)) {
                const IArrival *arrival = getArrival(receiverRadio, transmission);
                if (arrival->getEndTime() >= simTime()) {
                    cMethodCallContextSwitcher contextSwitcher(transmitterRadio);
                    contextSwitcher.methodCallSilent();
                    const cPacket *macFrame = transmission->getMacFrame();
                    EV_DEBUG << "Picking up " << macFrame << " originally sent "
                             << " from " << (IRadio *)transmitterRadio << " at " << transmission->getStartPosition()
                             << " to " << (IRadio *)receiverRadio << " at " << arrival->getStartPosition()
                             << " in " << arrival->getStartPropagationTime() * 1E+6 << " us propagation time." << endl;
                    TransmissionCacheEntry *transmissionCacheEntry = getTransmissionCacheEntry(transmission);
                    RadioFrame *frameCopy = dynamic_cast<const RadioFrame *>(transmissionCacheEntry->frame)->dup();
                    simtime_t delay = arrival->getStartTime() - simTime();
                    simtime_t duration = delay > 0 ? frameCopy->getDuration() : frameCopy->getDuration() + delay;
                    cGate *gate = receiverRadio->getRadioGate()->getPathStartGate();
                    const_cast<Radio *>(transmitterRadio)->sendDirect(frameCopy, delay > 0 ? delay : 0, duration, gate);
                    receptionCacheEntry->frame = frameCopy;
                    sendCount++;
                }
            }
        }
    }
}

void RadioMedium::updateCanvas()
{
    for (std::vector<const ITransmission *>::const_iterator it = transmissions.begin(); it != transmissions.end(); it++) {
        const ITransmission *transmission = *it;
        const TransmissionCacheEntry* transmissionCacheEntry = getTransmissionCacheEntry(transmission);
        cFigure *groupFigure = transmissionCacheEntry->figure;
        if (groupFigure) {
#if OMNETPP_CANVAS_VERSION >= 0x20140908
            cRingFigure *communicationFigure = (cRingFigure *)groupFigure->getFigure(0);
#else
            cOvalFigure *communicationFigure = (cOvalFigure *)groupFigure->getFigure(0);
#endif
            const Coord transmissionStart = transmission->getStartPosition();
            double startRadius = propagation->getPropagationSpeed().get() * (simTime() - transmission->getStartTime()).dbl();
            double endRadius = propagation->getPropagationSpeed().get() * (simTime() - transmission->getEndTime()).dbl();
            if (endRadius < 0) endRadius = 0;
            // KLUDGE: to workaround overflow bugs in drawing
            if (startRadius > 10000)
                startRadius = 10000;
            if (endRadius > 10000)
                endRadius = 10000;
#if OMNETPP_CANVAS_VERSION >= 0x20140908
            if (drawCommunication2D) {
                // determine the rotated 2D canvas points of the four corners of flat 3D circle's bounding box
                // it defines a 2D rotated parallelogram that needs to be filled with an oval
                cFigure::Point topLeft = PhysicalEnvironment::computeCanvasPoint(transmissionStart + Coord(-startRadius, -startRadius, 0));
                cFigure::Point topRight = PhysicalEnvironment::computeCanvasPoint(transmissionStart + Coord(startRadius, -startRadius, 0));
                cFigure::Point bottomLeft = PhysicalEnvironment::computeCanvasPoint(transmissionStart + Coord(-startRadius, startRadius, 0));
                cFigure::Point bottomRight = PhysicalEnvironment::computeCanvasPoint(transmissionStart + Coord(startRadius, startRadius, 0));
                cFigure::Point bottomDirectionVector = bottomLeft - bottomRight;
                LineSegment bottomHeight(Coord(topLeft.x, topLeft.y, 0), Coord(topLeft.x, topLeft.y, 0) + Coord(-bottomDirectionVector.y, bottomDirectionVector.x, 0));
                LineSegment bottomSide(Coord(bottomLeft.x, bottomLeft.y, 0), Coord(bottomRight.x, bottomRight.y, 0));
                Coord intersection1, intersection2;
                // TODO: use the real intersection
//                bool intersectionFound = heightLineSegment.computeIntersection(bottomLineSegment, intersection1, intersection2);
//                ASSERT(intersectionFound);
//                cFigure::Point intersection(intersection1.x, intersection1.y);
                // KLUDGE: for the simple case when the top and bottom sides are horizontal
                cFigure::Point bottomHeightIntersection(topLeft.x, bottomLeft.y);
                double width = (topLeft - topRight).getLength();
                double height = (topLeft - bottomHeightIntersection).getLength();
                cFigure::Point leftSideVector = topLeft - bottomLeft;
                cFigure::Point bottomSideVector = bottomRight - bottomLeft;
                double bottomRightCosAlpha = leftSideVector * bottomSideVector / leftSideVector.getLength() / bottomSideVector.getLength();
                double skewXAngle = acos(bottomRightCosAlpha);
                double rotationAngle = atan2(topRight.y - topLeft.y, topRight.x - topLeft.x);
                cFigure::Point center = PhysicalEnvironment::computeCanvasPoint(transmissionStart);
                communicationFigure->setTransform(cFigure::Transform());
                communicationFigure->skewx(-skewXAngle, center.y);
                communicationFigure->rotate(rotationAngle, center.x, center.y);
                communicationFigure->setBounds(cFigure::Rectangle(center.x - width / 2, center.y - height / 2, width, height));
                communicationFigure->setInnerRx(width * endRadius / startRadius / 2);
                communicationFigure->setInnerRy(height * endRadius / startRadius / 2);
                // TODO: delete debug code (this is the parallelogram where the oval should fit in)
//                std::cout << "ANGLE " << math::rad2deg(skewXAngle) << endl;
//                std::vector<cFigure::Point> points;
//                points.push_back(topLeft);
//                points.push_back(topRight);
//                points.push_back(bottomRight);
//                points.push_back(bottomLeft);
//                cPolygonFigure *f = new cPolygonFigure();
//                f->setLineColor(cFigure::RED);
//                f->setPoints(points);
//                communcationTrail->addFigure(f);
            }
            else {
#else
            {
#endif
                // a sphere looks like a circle from any view angle
                cFigure::Point center = PhysicalEnvironment::computeCanvasPoint(transmissionStart);
                communicationFigure->setBounds(cFigure::Rectangle(center.x - startRadius, center.y - startRadius, 2 * startRadius, 2 * startRadius));
#if OMNETPP_CANVAS_VERSION >= 0x20140908
                communicationFigure->setInnerRx(endRadius);
                communicationFigure->setInnerRy(endRadius);
#endif
            }
        }
    }
}

void RadioMedium::scheduleUpdateCanvasTimer()
{
    simtime_t now = simTime();
    for (std::vector<const ITransmission *>::const_iterator it = transmissions.begin(); it != transmissions.end(); it++)
    {
        const ITransmission *transmission = *it;
        const TransmissionCacheEntry *transmissionCacheEntry = getTransmissionCacheEntry(transmission);
        const simtime_t startTime = transmission->getStartTime();
        const simtime_t endTime = transmission->getEndTime();
        simtime_t maxPropagationTime = transmissionCacheEntry->interferenceEndTime - endTime - maxTransmissionDuration;
        if ((startTime <= now && now <= startTime + maxPropagationTime) ||
            (endTime <= now && now <= endTime + maxPropagationTime))
        {
            scheduleAt(simTime() + updateCanvasInterval, updateCanvasTimer);
            return;
        }
    }
    simtime_t nextEndTime = -1;
    for (std::vector<const ITransmission *>::const_iterator it = transmissions.begin(); it != transmissions.end(); it++) {
        const ITransmission *transmission = *it;
        const simtime_t endTime = transmission->getEndTime();
        if (endTime > now && (nextEndTime == -1 || endTime < nextEndTime))
            nextEndTime = endTime;
    }
    if (nextEndTime > now)
        scheduleAt(nextEndTime, updateCanvasTimer);
}

Coord RadioMedium::computeConstraintAreaMin() const
{
    Coord constraintAreaMin = Coord::NIL;
    if (radios.size() > 0)
        constraintAreaMin = radios[0]->getAntenna()->getMobility()->getConstraintAreaMin();
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
    {
        const IMobility *mobility = (*it)->getAntenna()->getMobility();
        Coord currConstreaintAreaMin = mobility->getConstraintAreaMin();

        if (constraintAreaMin.x > currConstreaintAreaMin.x)
            constraintAreaMin.x = currConstreaintAreaMin.x;
        if (constraintAreaMin.y > currConstreaintAreaMin.y)
            constraintAreaMin.y = currConstreaintAreaMin.y;
        if (constraintAreaMin.z > currConstreaintAreaMin.z)
            constraintAreaMin.z = currConstreaintAreaMin.z;
    }
    return constraintAreaMin;
}

Coord RadioMedium::computeConstreaintAreaMax() const
{
    Coord constraintAreaMax = Coord::NIL;
    if (radios.size() > 0)
        constraintAreaMax = radios[0]->getAntenna()->getMobility()->getConstraintAreaMax();
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
    {
        const IMobility *mobility = (*it)->getAntenna()->getMobility();
        Coord currConstraintAreaMax = mobility->getConstraintAreaMax();

        if (constraintAreaMax.x < currConstraintAreaMax.x)
            constraintAreaMax.x = currConstraintAreaMax.x;
        if (constraintAreaMax.y < currConstraintAreaMax.y)
            constraintAreaMax.y = currConstraintAreaMax.y;
        if (constraintAreaMax.z < currConstraintAreaMax.z)
            constraintAreaMax.z = currConstraintAreaMax.z;
    }
    return constraintAreaMax;
}

m RadioMedium::getMaxInterferenceRange(const IRadio* radio) const
{
    m maxInterferenceRange = computeMaxRange(radio->getTransmitter()->getMaxPower(), minInterferencePower);
    if (!isNaN(maxInterferenceRange.get()))
        return maxInterferenceRange;
    return radio->getTransmitter()->getMaxInterferenceRange();
}

m RadioMedium::getMaxCommunicationRange(const IRadio* radio) const
{
    m maxCommunicationRange = computeMaxRange(radio->getTransmitter()->getMaxPower(), minReceptionPower);
    if (!isNaN(maxCommunicationRange.get()))
        return maxCommunicationRange;
    return radio->getTransmitter()->getMaxCommunicationRange();
}

} // namespace physicallayer

} // namespace inet

