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

#include "Radio.h"
#include "RadioChannel.h"
#include "PhyControlInfo_m.h"
#include "IMACFrame.h"
#include "InterfaceTableAccess.h"

Define_Module(RadioChannel);

RadioChannel::RadioChannel() :
    propagation(NULL),
    attenuation(NULL),
    backgroundNoise(NULL),
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
    recordCommunication(false),
    removeNonInterferingTransmissionsTimer(NULL),
    baseRadioId(0),
    baseTransmissionId(0),
    transmissionCount(0),
    receptionComputationCount(0),
    receptionDecisionComputationCount(0),
    listeningDecisionComputationCount(0),
    cacheReceptionGetCount(0),
    cacheReceptionHitCount(0),
    cacheDecisionGetCount(0),
    cacheDecisionHitCount(0)
{
}

RadioChannel::RadioChannel(const IRadioSignalPropagation *propagation, const IRadioSignalAttenuation *attenuation, const IRadioBackgroundNoise *backgroundNoise, const simtime_t minInterferenceTime, const simtime_t maxTransmissionDuration, m maxCommunicationRange, m maxInterferenceRange) :
    propagation(propagation),
    attenuation(attenuation),
    backgroundNoise(backgroundNoise),
    maxTransmissionPower(W(sNaN)),
    minInterferencePower(W(sNaN)),
    minReceptionPower(W(sNaN)),
    maxAntennaGain(sNaN),
    minInterferenceTime(minInterferenceTime),
    maxTransmissionDuration(maxTransmissionDuration),
    maxCommunicationRange(m(maxCommunicationRange)),
    maxInterferenceRange(m(maxInterferenceRange)),
    rangeFilter(RANGE_FILTER_ANYWHERE),
    radioModeFilter(false),
    listeningFilter(false),
    macAddressFilter(false),
    recordCommunication(false),
    removeNonInterferingTransmissionsTimer(NULL),
    baseRadioId(0),
    baseTransmissionId(0),
    transmissionCount(0),
    receptionComputationCount(0),
    receptionDecisionComputationCount(0),
    listeningDecisionComputationCount(0),
    cacheReceptionGetCount(0),
    cacheReceptionHitCount(0),
    cacheDecisionGetCount(0),
    cacheDecisionHitCount(0)
{
}

RadioChannel::~RadioChannel()
{
    delete propagation;
    delete attenuation;
    delete backgroundNoise;
    for (std::vector<const IRadioSignalTransmission *>::const_iterator it = transmissions.begin(); it != transmissions.end(); it++)
        delete *it;
    cancelAndDelete(removeNonInterferingTransmissionsTimer);
    for (std::vector<TransmissionCacheEntry>::const_iterator it = cache.begin(); it != cache.end(); it++)
    {
        const std::vector<ReceptionCacheEntry> *cacheEntries = (*it).receptionCacheEntries;
        if (cacheEntries)
        {
            for (std::vector<ReceptionCacheEntry>::const_iterator jt = cacheEntries->begin(); jt != cacheEntries->end(); jt++)
            {
                const ReceptionCacheEntry &cacheEntry = *jt;
                delete cacheEntry.arrival;
                delete cacheEntry.reception;
                delete cacheEntry.decision;
            }
            delete cacheEntries;
        }
    }
    if (recordCommunication)
        communicationLog.close();
}

void RadioChannel::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        // initialize parameters
        propagation = check_and_cast<IRadioSignalPropagation *>(getSubmodule("propagation"));
        attenuation = check_and_cast<IRadioSignalAttenuation *>(getSubmodule("attenuation"));
        backgroundNoise = dynamic_cast<IRadioBackgroundNoise *>(getSubmodule("backgroundNoise"));
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
        removeNonInterferingTransmissionsTimer = new cMessage("removeNonInterferingTransmissionsTimer");
        // initialize logging
        recordCommunication = par("recordCommunication");
        if (recordCommunication)
            communicationLog.open(ev.getConfig()->substituteVariables("${resultdir}/${configname}-${runnumber}.tlog"));
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER)
    {
        updateLimits();
    }
    else if (stage == INITSTAGE_LAST)
    {
        EV_DEBUG << "Radio channel initialized"
                 << ", maximum transmission power = " << maxTransmissionPower
                 << ", minimum interference power = " << minInterferencePower
                 << ", minimum reception power = " << minReceptionPower
                 << ", maximum antenna gain = " << maxAntennaGain
                 << ", minimum interference time = " << minInterferenceTime << " s"
                 << ", maximum transmission duration = " << maxTransmissionDuration << " s"
                 << ", maximum communication range = " << maxCommunicationRange
                 << ", maximum interference range = " << maxInterferenceRange
                 << ", " << propagation << ", " << attenuation << ", " << backgroundNoise << endl;
    }
}


void RadioChannel::finish()
{
    double receptionCacheHitPercentage = 100 * (double)cacheReceptionHitCount / (double)cacheReceptionGetCount;
    double decisionCacheHitPercentage = 100 * (double)cacheDecisionHitCount / (double)cacheDecisionGetCount;
    EV_INFO << "Radio channel transmission count = " << transmissionCount << endl;
    EV_INFO << "Radio channel reception computation count = " << receptionComputationCount << endl;
    EV_INFO << "Radio channel reception decision computation count = " << receptionDecisionComputationCount << endl;
    EV_INFO << "Radio channel listening decision computation count = " << listeningDecisionComputationCount << endl;
    EV_INFO << "Radio channel reception cache hit = " << receptionCacheHitPercentage << " %" << endl;
    EV_INFO << "Radio channel reception decision cache hit = " << decisionCacheHitPercentage << " %" << endl;
    recordScalar("Radio channel transmission count", transmissionCount);
    recordScalar("Radio channel reception computation count", receptionComputationCount);
    recordScalar("Radio channel reception decision computation count", receptionDecisionComputationCount);
    recordScalar("Radio channel listening decision computation count", listeningDecisionComputationCount);
    recordScalar("Radio channel reception cache hit", receptionCacheHitPercentage, "%");
    recordScalar("Radio channel reception decision cache hit", decisionCacheHitPercentage, "%");
}

void RadioChannel::handleMessage(cMessage *message)
{
    if (message == removeNonInterferingTransmissionsTimer)
        removeNonInterferingTransmissions();
    else
        throw cRuntimeError("Unknown message");
}

RadioChannel::TransmissionCacheEntry *RadioChannel::getTransmissionCacheEntry(const IRadioSignalTransmission *transmission) const
{
    size_t transmissionIndex = transmission->getId() - baseTransmissionId;
    if (transmissionIndex < 0)
        return NULL;
    else
    {
        if (transmissionIndex >= cache.size())
            cache.resize(transmissionIndex + 1);
        TransmissionCacheEntry &transmissionCacheEntry = cache[transmissionIndex];
        if (!transmissionCacheEntry.receptionCacheEntries)
            transmissionCacheEntry.receptionCacheEntries = new std::vector<ReceptionCacheEntry>(radios.size());
        return &transmissionCacheEntry;
    }
}

RadioChannel::ReceptionCacheEntry *RadioChannel::getReceptionCacheEntry(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    TransmissionCacheEntry *transmissionCacheEntry = getTransmissionCacheEntry(transmission);
    if (!transmissionCacheEntry)
        return NULL;
    else
    {
        std::vector<ReceptionCacheEntry> *cacheEntries = transmissionCacheEntry->receptionCacheEntries;
        size_t radioIndex = radio->getId() - baseRadioId;
        if (radioIndex < 0)
            return NULL;
        else
        {
            if (radioIndex >= cacheEntries->size())
                cacheEntries->resize(radioIndex + 1);
            return &(*cacheEntries)[radioIndex];
        }
    }
}

const IRadioSignalArrival *RadioChannel::getCachedArrival(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry  ? cacheEntry->arrival : NULL;
}

void RadioChannel::setCachedArrival(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalArrival *arrival) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->arrival = arrival;
}

void RadioChannel::removeCachedArrival(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->arrival = NULL;
}

const IRadioSignalReception *RadioChannel::getCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->reception : NULL;
}

void RadioChannel::setCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalReception *reception) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->reception = reception;
}

void RadioChannel::removeCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->reception = NULL;
}

const IRadioSignalReceptionDecision *RadioChannel::getCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->decision : NULL;
}

void RadioChannel::setCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalReceptionDecision *decision) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->decision = decision;
}

void RadioChannel::removeCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->decision = NULL;
}

void RadioChannel::invalidateCachedDecisions(const IRadioSignalTransmission *transmission)
{
    for (std::vector<TransmissionCacheEntry>::iterator it = cache.begin(); it != cache.end(); it++)
    {
        std::vector<ReceptionCacheEntry> *cacheEntries = (*it).receptionCacheEntries;
        if (cacheEntries)
        {
            for (std::vector<ReceptionCacheEntry>::iterator jt = cacheEntries->begin(); jt != cacheEntries->end(); jt++)
            {
                ReceptionCacheEntry &cacheEntry = *jt;
                const IRadioSignalReceptionDecision *decision = cacheEntry.decision;
                if (decision)
                {
                    const IRadioSignalReception *reception = decision->getReception();
                    if (isInterferingTransmission(transmission, reception))
                        invalidateCachedDecision(decision);
                }
            }
        }
    }
}

void RadioChannel::invalidateCachedDecision(const IRadioSignalReceptionDecision *decision)
{
    const IRadioSignalReception *reception = decision->getReception();
    const IRadio *radio = reception->getReceiver();
    const IRadioSignalTransmission *transmission = reception->getTransmission();
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->decision = NULL;
}

template<typename T> inline T minIgnoreNaN(T a, T b)
{
    if (isNaN(a.get())) return b;
    else if (isNaN(b.get())) return a;
    else if (a < b) return a;
    else return b;
}

template<typename T> inline T maxIgnoreNaN(T a, T b)
{
    if (isNaN(a.get())) return b;
    else if (isNaN(b.get())) return a;
    else if (a > b) return a;
    else return b;
}

inline double maxIgnoreNaN(double a, double b)
{
    if (isNaN(a)) return b;
    else if (isNaN(b)) return a;
    else if (a > b) return a;
    else return b;
}

W RadioChannel::computeMaxTransmissionPower() const
{
    W maxTransmissionPower = W(par("maxTransmissionPower"));
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
        maxTransmissionPower = maxIgnoreNaN(maxTransmissionPower, (*it)->getTransmitter()->getMaxPower());
    return maxTransmissionPower;
}

W RadioChannel::computeMinInterferencePower() const
{
    W minInterferencePower = mW(FWMath::dBm2mW(par("minInterferencePower")));
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
        minInterferencePower = minIgnoreNaN(minInterferencePower, (*it)->getReceiver()->getMinInterferencePower());
    return minInterferencePower;
}

W RadioChannel::computeMinReceptionPower() const
{
    W minReceptionPower = mW(FWMath::dBm2mW(par("minReceptionPower")));
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
        minReceptionPower = minIgnoreNaN(minReceptionPower, (*it)->getReceiver()->getMinReceptionPower());
    return minReceptionPower;
}

double RadioChannel::computeMaxAntennaGain() const
{
    double maxAntennaGain = FWMath::dB2fraction(par("maxAntennaGain"));
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
        maxAntennaGain = maxIgnoreNaN(maxAntennaGain, (*it)->getAntenna()->getMaxGain());
    return maxAntennaGain;
}

m RadioChannel::computeMaxRange(W maxTransmissionPower, W minReceptionPower) const
{
    double alpha = par("alpha");
    Hz carrierFrequency = Hz(par("carrierFrequency"));
    m waveLength = mps(SPEED_OF_LIGHT) / carrierFrequency;
    double minFactor = (minReceptionPower / maxAntennaGain / maxAntennaGain / maxTransmissionPower).get();
    return waveLength / pow(minFactor * 16.0 * M_PI * M_PI, 1.0 / alpha);
}

m RadioChannel::computeMaxCommunicationRange() const
{
    return maxIgnoreNaN(m(par("maxCommunicationRange")), computeMaxRange(maxTransmissionPower, minReceptionPower));
}

m RadioChannel::computeMaxInterferenceRange() const
{
    return maxIgnoreNaN(m(par("maxInterferenceRange")), computeMaxRange(maxTransmissionPower, minInterferencePower));
}

const simtime_t RadioChannel::computeMinInterferenceTime() const
{
    return par("minInterferenceTime").doubleValue();
}

const simtime_t RadioChannel::computeMaxTransmissionDuration() const
{
    return par("maxTransmissionDuration").doubleValue();
}

void RadioChannel::updateLimits()
{
    maxTransmissionPower = computeMaxTransmissionPower();
    minInterferencePower = computeMinInterferencePower();
    minReceptionPower = computeMinReceptionPower();
    maxAntennaGain = computeMaxAntennaGain();
    minInterferenceTime = computeMinInterferenceTime();
    maxTransmissionDuration = computeMaxTransmissionDuration();
    maxCommunicationRange = computeMaxCommunicationRange();
    maxInterferenceRange = computeMaxInterferenceRange();
}

bool RadioChannel::isRadioMacAddress(const IRadio *radio, const MACAddress address) const
{
    IInterfaceTable *interfaceTable = InterfaceTableAccess().get(const_cast<cModule *>(check_and_cast<const cModule *>(radio)));
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
    {
        const InterfaceEntry *interface = interfaceTable->getInterface(i);
        if (interface && interface->getMacAddress() == address)
            return true;
    }
    return false;
}

bool RadioChannel::isInCommunicationRange(const IRadioSignalTransmission *transmission, const Coord startPosition, const Coord endPosition) const
{
    return isNaN(maxCommunicationRange.get()) ||
           (transmission->getStartPosition().distance(startPosition) < maxCommunicationRange.get() &&
            transmission->getEndPosition().distance(endPosition) < maxCommunicationRange.get());
}

bool RadioChannel::isInInterferenceRange(const IRadioSignalTransmission *transmission, const Coord startPosition, const Coord endPosition) const
{
    return isNaN(maxInterferenceRange.get()) ||
           (transmission->getStartPosition().distance(startPosition) < maxInterferenceRange.get() &&
            transmission->getEndPosition().distance(endPosition) < maxInterferenceRange.get());
}

bool RadioChannel::isInterferingTransmission(const IRadioSignalTransmission *transmission, const IRadioSignalListening *listening) const
{
    const IRadio *transmitter = transmission->getTransmitter();
    const IRadio *receiver = listening->getReceiver();
    const IRadioSignalArrival *arrival = getArrival(receiver, transmission);
    return transmitter != receiver &&
           arrival->getEndTime() >= listening->getStartTime() + minInterferenceTime &&
           arrival->getStartTime() <= listening->getEndTime() - minInterferenceTime &&
           isInInterferenceRange(transmission, listening->getStartPosition(), listening->getEndPosition());
}

bool RadioChannel::isInterferingTransmission(const IRadioSignalTransmission *transmission, const IRadioSignalReception *reception) const
{
    const IRadio *transmitter = transmission->getTransmitter();
    const IRadio *receiver = reception->getReceiver();
    const IRadioSignalArrival *arrival = getArrival(receiver, transmission);
    return transmitter != receiver &&
           arrival->getEndTime() > reception->getStartTime() + minInterferenceTime &&
           arrival->getStartTime() < reception->getEndTime() - minInterferenceTime &&
           isInInterferenceRange(transmission, reception->getStartPosition(), reception->getEndPosition());
}

void RadioChannel::removeNonInterferingTransmissions()
{
    const simtime_t now = simTime();
    size_t transmissionIndex = 0;
    while (transmissionIndex < cache.size() && cache[transmissionIndex].interferenceEndTime <= now)
        transmissionIndex++;
    EV_DEBUG << "Removing " << transmissionIndex << " non interfering transmissions\n";
    baseTransmissionId += transmissionIndex;
    cache.erase(cache.begin(), cache.begin() + transmissionIndex);
    transmissions.erase(transmissions.begin(), transmissions.begin() + transmissionIndex);
    if (cache.size() > 0)
        scheduleAt(cache[0].interferenceEndTime, removeNonInterferingTransmissionsTimer);
}

const IRadioSignalReception *RadioChannel::computeReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    receptionComputationCount++;
    return attenuation->computeReception(radio, transmission);
}

const std::vector<const IRadioSignalReception *> *RadioChannel::computeInterferingReceptions(const IRadioSignalListening *listening, const std::vector<const IRadioSignalTransmission *> *transmissions) const
{
    const IRadio *radio = listening->getReceiver();
    std::vector<const IRadioSignalReception *> *interferingReceptions = new std::vector<const IRadioSignalReception *>();
    for (std::vector<const IRadioSignalTransmission *>::const_iterator it = transmissions->begin(); it != transmissions->end(); it++)
    {
        const IRadioSignalTransmission *interferingTransmission = *it;
        if (interferingTransmission->getTransmitter() != radio && isInterferingTransmission(interferingTransmission, listening))
            interferingReceptions->push_back(getReception(radio, interferingTransmission));
    }
    return interferingReceptions;
}

const std::vector<const IRadioSignalReception *> *RadioChannel::computeInterferingReceptions(const IRadioSignalReception *reception, const std::vector<const IRadioSignalTransmission *> *transmissions) const
{
    const IRadio *radio = reception->getReceiver();
    const IRadioSignalTransmission *transmission = reception->getTransmission();
    std::vector<const IRadioSignalReception *> *interferingReceptions = new std::vector<const IRadioSignalReception *>();
    for (std::vector<const IRadioSignalTransmission *>::const_iterator it = transmissions->begin(); it != transmissions->end(); it++)
    {
        const IRadioSignalTransmission *interferingTransmission = *it;
        if (transmission != interferingTransmission && interferingTransmission->getTransmitter() != radio && isInterferingTransmission(interferingTransmission, reception))
            interferingReceptions->push_back(getReception(radio, interferingTransmission));
    }
    return interferingReceptions;
}

const IRadioSignalReceptionDecision *RadioChannel::computeReceptionDecision(const IRadio *radio, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission, const std::vector<const IRadioSignalTransmission *> *transmissions) const
{
    receptionDecisionComputationCount++;
    const IRadioSignalReception *reception = getReception(radio, transmission);
    const std::vector<const IRadioSignalReception *> *interferingReceptions = computeInterferingReceptions(reception, transmissions);
    const IRadioSignalNoise *noise = backgroundNoise ? backgroundNoise->computeNoise(reception) : NULL;
    const IRadioSignalReceptionDecision *decision = radio->getReceiver()->computeReceptionDecision(listening, reception, interferingReceptions, noise);
    delete noise;
    delete interferingReceptions;
    return decision;
}

const IRadioSignalListeningDecision *RadioChannel::computeListeningDecision(const IRadio *radio, const IRadioSignalListening *listening, const std::vector<const IRadioSignalTransmission *> *transmissions) const
{
    listeningDecisionComputationCount++;
    const std::vector<const IRadioSignalReception *> *interferingReceptions = computeInterferingReceptions(listening, transmissions);
    const IRadioSignalNoise *noise = backgroundNoise ? backgroundNoise->computeNoise(listening) : NULL;
    const IRadioSignalListeningDecision *decision = radio->getReceiver()->computeListeningDecision(listening, interferingReceptions, noise);
    delete noise;
    delete interferingReceptions;
    return decision;
}

const IRadioSignalReception *RadioChannel::getReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    cacheReceptionGetCount++;
    const IRadioSignalReception *reception = getCachedReception(radio, transmission);
    if (reception)
        cacheReceptionHitCount++;
    else
    {
        reception = computeReception(radio, transmission);
        setCachedReception(radio, transmission, reception);
        EV_DEBUG << "Receiving " << transmission << " from channel by " << radio << " arrives as " << reception << endl;
    }
    return reception;
}

const IRadioSignalReceptionDecision *RadioChannel::getReceptionDecision(const IRadio *radio, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission) const
{
    cacheDecisionGetCount++;
    const IRadioSignalReceptionDecision *decision = getCachedDecision(radio, transmission);
    if (decision)
        cacheDecisionHitCount++;
    else
    {
        decision = computeReceptionDecision(radio, listening, transmission, const_cast<const std::vector<const IRadioSignalTransmission *> *>(&transmissions));
        setCachedDecision(radio, transmission, decision);
        EV_DEBUG << "Receiving " << transmission << " from channel by " << radio << " arrives as " << decision->getReception() << " and results in " << decision << endl;
    }
    return decision;
}

void RadioChannel::addRadio(const IRadio *radio)
{
    radios.push_back(radio);
    for (std::vector<const IRadioSignalTransmission *>::const_iterator it = transmissions.begin(); it != transmissions.end(); it++)
    {
        const IRadioSignalTransmission *transmission = *it;
        const IRadioSignalArrival *arrival = propagation->computeArrival(transmission, radio->getAntenna()->getMobility());
        setCachedArrival(radio, transmission, arrival);
    }
    if (initialized())
        updateLimits();
    cModule *radioModule = const_cast<cModule *>(check_and_cast<const cModule *>(radio));
    if (radioModeFilter)
        radioModule->subscribe(OldIRadio::radioModeChangedSignal, this);
    if (listeningFilter)
        radioModule->subscribe(OldIRadio::listeningChangedSignal, this);
}

void RadioChannel::removeRadio(const IRadio *radio)
{
    radios.erase(std::remove(radios.begin(), radios.end(), radio));
    // TODO: remove all references to radio and erase entries from cache
    if (initialized())
        updateLimits();
}

void RadioChannel::transmitToChannel(const IRadio *transmitterRadio, const IRadioSignalTransmission *transmission)
{
    transmissionCount++;
    transmissions.push_back(transmission);
    simtime_t maxArrivalEndTime = simTime();
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
    {
        const IRadio *receiverRadio = *it;
        if (receiverRadio != transmitterRadio)
        {
            const IRadioSignalArrival *arrival = propagation->computeArrival(transmission, receiverRadio->getAntenna()->getMobility());
            const simtime_t arrivalEndTime = arrival->getEndTime();
            if (arrivalEndTime > maxArrivalEndTime)
                maxArrivalEndTime = arrivalEndTime;
            setCachedArrival(receiverRadio, transmission, arrival);
        }
    }
    getTransmissionCacheEntry(transmission)->interferenceEndTime = maxArrivalEndTime + maxTransmissionDuration;
    if (!removeNonInterferingTransmissionsTimer->isScheduled())
        scheduleAt(cache[0].interferenceEndTime, removeNonInterferingTransmissionsTimer);
}

void RadioChannel::sendToChannel(IRadio *radio, const IRadioFrame *frame)
{
    const Radio *transmitterRadio = check_and_cast<const Radio *>(radio);
    const RadioFrame *radioFrame = check_and_cast<const RadioFrame *>(frame);
    const IRadioSignalTransmission *transmission = frame->getTransmission();
    EV_DEBUG << "Sending " << frame << " with " << radioFrame->getBitLength() << " bits in " << radioFrame->getDuration() * 1E+6 << " us transmission duration"
             << " from " << radio << " on " << this << "." << endl;
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
    {
        const Radio *receiverRadio = check_and_cast<const Radio *>(*it);
        ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiverRadio, transmission);
        if (receiverRadio != transmitterRadio && isPotentialReceiver(receiverRadio, transmission))
        {
            cGate *gate = receiverRadio->RadioBase::getRadioGate()->getPathStartGate();
            const IRadioSignalArrival *arrival = getArrival(receiverRadio, transmission);
            simtime_t propagationTime = arrival->getStartPropagationTime();
            EV_DEBUG << "Sending " << frame
                     << " from " << (IRadio *)transmitterRadio << " at " << transmission->getStartPosition()
                     << " to " << (IRadio *)receiverRadio << " at " << arrival->getStartPosition()
                     << " in " << propagationTime * 1E+6 << " us propagation time." << endl;
            RadioFrame *frameCopy = new RadioFrame(radioFrame->getTransmission());
            frameCopy->setName(radioFrame->getName());
            frameCopy->setDuration(radioFrame->getDuration());
            frameCopy->encapsulate(radioFrame->getEncapsulatedPacket()->dup());
            const_cast<Radio *>(transmitterRadio)->sendDirect(frameCopy, propagationTime, radioFrame->getDuration(), gate);
            cacheEntry->isRadioFrameSent = true;
        }
    }
}

IRadioFrame *RadioChannel::transmitPacket(const IRadio *radio, cPacket *macFrame)
{
    const IRadioSignalTransmission *transmission = radio->getTransmitter()->createTransmission(radio, macFrame, simTime());
    transmitToChannel(radio, transmission);
    RadioFrame *radioFrame = new RadioFrame(transmission);
    radioFrame->setName(macFrame->getName());
    radioFrame->setDuration(transmission->getEndTime() - transmission->getStartTime());
    radioFrame->encapsulate(macFrame);
    if (recordCommunication) {
        const Radio *transmitterRadio = check_and_cast<const Radio *>(radio);
        communicationLog << "T " << transmitterRadio->getFullPath() << " " << transmitterRadio->getId() << " "
                         << "M " << radioFrame->getName() << " " << transmission->getId() << " "
                         << "S " << transmission->getStartTime() << " " <<  transmission->getStartPosition() << " -> "
                         << "D " << transmission->getEndTime() << " " <<  transmission->getEndPosition() << endl;
    }
    return radioFrame;
}

cPacket *RadioChannel::receivePacket(const IRadio *radio, IRadioFrame *radioFrame)
{
    const IRadioSignalTransmission *transmission = radioFrame->getTransmission();
    const IRadioSignalListening *listening = radio->getReceiver()->createListening(radio, transmission->getStartTime(), transmission->getEndTime(), transmission->getStartPosition(), transmission->getEndPosition());
    const IRadioSignalReceptionDecision *decision = receiveFromChannel(radio, listening, transmission);
    if (recordCommunication) {
        const IRadioSignalReception *reception = decision->getReception();
        communicationLog << "R " << check_and_cast<const Radio *>(radio)->getFullPath() << " " << reception->getReceiver()->getId() << " "
                         << "M " << check_and_cast<const RadioFrame *>(radioFrame)->getName() << " " << transmission->getId() << " "
                         << "S " << reception->getStartTime() << " " <<  reception->getStartPosition() << " -> "
                         << "D " << reception->getEndTime() << " " <<  reception->getEndPosition() << " "
                         << "F " << decision->isReceptionPossible() << " " << decision->isReceptionAttempted() << " " << decision->isReceptionSuccessful() << endl;
    }
    cPacket *macFrame = check_and_cast<cPacket *>(radioFrame)->decapsulate();
    if (!decision->isReceptionSuccessful())
        macFrame->setKind(decision->getBitErrorCount() > 0 ? BITERROR : COLLISION);
    macFrame->setControlInfo(const_cast<cObject *>(check_and_cast<const cObject *>(decision)));
    delete listening;
    return macFrame;
}

const IRadioSignalReceptionDecision *RadioChannel::receiveFromChannel(const IRadio *radio, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission) const
{
    const IRadioSignalReceptionDecision *decision = getReceptionDecision(radio, listening, transmission);
    removeCachedDecision(radio, transmission);
    return decision;
}

const IRadioSignalListeningDecision *RadioChannel::listenOnChannel(const IRadio *radio, const IRadioSignalListening *listening) const
{
    const IRadioSignalListeningDecision *decision = computeListeningDecision(radio, listening, const_cast<const std::vector<const IRadioSignalTransmission *> *>(&transmissions));
    EV_DEBUG << "Listening " << listening << " on channel by " << radio << " results in " << decision << endl;
    return decision;
}

bool RadioChannel::isPotentialReceiver(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    const Radio *receiverRadio = check_and_cast<const Radio *>(radio);
    if (radioModeFilter && receiverRadio->getRadioMode() != OldIRadio::RADIO_MODE_RECEIVER && receiverRadio->getRadioMode() != OldIRadio::RADIO_MODE_TRANSCEIVER)
        return false;
    else if (listeningFilter && !radio->getReceiver()->computeIsReceptionPossible(transmission))
        return false;
    else if (macAddressFilter && !isRadioMacAddress(radio, check_and_cast<const IMACFrame *>(transmission->getMacFrame())->getDestinationAddress()))
        return false;
    else if (rangeFilter == RANGE_FILTER_INTERFERENCE_RANGE) {
        const IRadioSignalArrival *arrival = getArrival(radio, transmission);
        return isInInterferenceRange(transmission, arrival->getStartPosition(), arrival->getEndPosition());
    }
    else if (rangeFilter == RANGE_FILTER_COMMUNICATION_RANGE) {
        const IRadioSignalArrival *arrival = getArrival(radio, transmission);
        return isInCommunicationRange(transmission, arrival->getStartPosition(), arrival->getEndPosition());
    }
    else
        return true;
}

bool RadioChannel::isReceptionAttempted(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    const IRadioSignalReception *reception = getReception(radio, transmission);
    // TODO: isn't there a better way for this optimization? see also in RadioSignalReceiverBase::computeIsReceptionAttempted
    const std::vector<const IRadioSignalReception *> *interferingReceptions = simTime() == reception->getStartTime() ? NULL : computeInterferingReceptions(reception, const_cast<const std::vector<const IRadioSignalTransmission *> *>(&transmissions));
    bool isReceptionAttempted = radio->getReceiver()->computeIsReceptionAttempted(reception, interferingReceptions);
    delete interferingReceptions;
    EV_DEBUG << "Receiving " << transmission << " from channel by " << radio << " arrives as " << reception << " and results in reception is " << (isReceptionAttempted ? "attempted" : "ignored") << endl;
    return isReceptionAttempted;
}

const IRadioSignalArrival *RadioChannel::getArrival(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    return getCachedArrival(radio, transmission);
}

void RadioChannel::receiveSignal(cComponent *source, simsignal_t signal, long value)
{
    Enter_Method_Silent();
    if (signal == OldIRadio::radioModeChangedSignal || signal == OldIRadio::listeningChangedSignal)
    {
        const Radio *receiverRadio = check_and_cast<const Radio *>(source);
        for (std::vector<const IRadioSignalTransmission *>::iterator it = transmissions.begin(); it != transmissions.end();)
        {
            const IRadioSignalTransmission *transmission = *it;
            const Radio *transmitterRadio = check_and_cast<const Radio *>(transmission->getTransmitter());
            ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiverRadio, transmission);
            if (cacheEntry && !cacheEntry->isRadioFrameSent && receiverRadio != transmitterRadio && isPotentialReceiver(receiverRadio, transmission))
            {
                const cPacket *macFrame = transmission->getMacFrame();
                const IRadioSignalArrival *arrival = getArrival(receiverRadio, transmission);
                EV_DEBUG << "Picking up " << macFrame << " originally sent "
                         << " from " << (IRadio *)transmitterRadio << " at " << transmission->getStartPosition()
                         << " to " << (IRadio *)receiverRadio << " at " << arrival->getStartPosition()
                         << " in " << arrival->getStartPropagationTime() * 1E+6 << " us propagation time." << endl;
                RadioFrame *radioFrame = new RadioFrame(transmission);
                radioFrame->setName(macFrame->getName());
                radioFrame->setDuration(transmission->getEndTime() - transmission->getStartTime());
                radioFrame->encapsulate(macFrame->dup());
                cGate *gate = receiverRadio->RadioBase::getRadioGate()->getPathStartGate();
                simtime_t delay = arrival->getStartTime() - simTime();
                const_cast<Radio *>(transmitterRadio)->sendDirect(radioFrame, delay > 0 ? delay : 0, radioFrame->getDuration(), gate);
                cacheEntry->isRadioFrameSent = true;
            }
        }
    }
}
