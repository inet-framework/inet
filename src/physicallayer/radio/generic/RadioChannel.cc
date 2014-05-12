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
// TODO: should not be here
#include "ScalarImplementation.h"
#include "PhyControlInfo_m.h"

Define_Module(RadioChannel);

RadioChannel::RadioChannel() :
    propagation(NULL),
    attenuation(NULL),
    backgroundNoise(NULL),
    minInterferenceTime(sNaN),
    maxTransmissionDuration(sNaN),
    maxCommunicationRange(m(sNaN)),
    maxInterferenceRange(m(sNaN)),
    recordCommunication(false),
    baseTransmissionId(0),
    lastRemoveNonInterferingTransmissions(0),
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
    minInterferenceTime(minInterferenceTime),
    maxTransmissionDuration(maxTransmissionDuration),
    maxCommunicationRange(m(maxCommunicationRange)),
    maxInterferenceRange(m(maxInterferenceRange)),
    recordCommunication(false),
    baseTransmissionId(0),
    lastRemoveNonInterferingTransmissions(0),
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
    for (std::vector<std::vector<CacheEntry> *>::const_iterator it = cache.begin(); it != cache.end(); it++)
    {
        const std::vector<CacheEntry> *cacheEntries = *it;
        if (cacheEntries)
        {
            for (std::vector<CacheEntry>::const_iterator jt = cacheEntries->begin(); jt != cacheEntries->end(); jt++)
            {
                const CacheEntry &cacheEntry = *jt;
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
        minInterferenceTime = computeMinInterferenceTime();
        maxTransmissionDuration = computeMaxTransmissionDuration();
        maxCommunicationRange = computeMaxCommunicationRange();
        maxInterferenceRange = computeMaxInterferenceRange();
        propagation = check_and_cast<IRadioSignalPropagation *>(getSubmodule("propagation"));
        attenuation = check_and_cast<IRadioSignalAttenuation *>(getSubmodule("attenuation"));
        backgroundNoise = dynamic_cast<IRadioBackgroundNoise *>(getSubmodule("backgroundNoise"));
        recordCommunication = par("recordCommunication");
        if (recordCommunication)
            communicationLog.open(ev.getConfig()->substituteVariables("${resultdir}/${configname}-${runnumber}.tlog"));
    }
    else if (stage == INITSTAGE_LAST)
    {
        EV_DEBUG << "Radio channel initialized with"
                 << " minimum interference time = " << minInterferenceTime << " s"
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

RadioChannel::CacheEntry *RadioChannel::getCacheEntry(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    int transmissionId = transmission->getId();
    int transmissionIndex = transmissionId - baseTransmissionId;
    if (transmissionIndex < 0)
        return NULL;
    else
    {
        if (transmissionIndex >= (int)cache.size())
            cache.resize(transmissionIndex + 1);
        std::vector<CacheEntry> *cacheEntries = cache[transmissionIndex];
        if (!cacheEntries)
            cacheEntries = cache[transmissionIndex] = new std::vector<CacheEntry>(radios.size());
        int radioId = radio->getId();
        if (radioId >= (int)cacheEntries->size())
            cacheEntries->resize(radioId + 1);
        return &(*cacheEntries)[radioId];
    }
}

const IRadioSignalArrival *RadioChannel::getCachedArrival(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    CacheEntry *cacheEntry = getCacheEntry(radio, transmission);
    return cacheEntry  ? cacheEntry->arrival : NULL;
}

void RadioChannel::setCachedArrival(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalArrival *arrival) const
{
    CacheEntry *cacheEntry = getCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->arrival = arrival;
}

void RadioChannel::removeCachedArrival(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    CacheEntry *cacheEntry = getCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->arrival = NULL;
}

const IRadioSignalReception *RadioChannel::getCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    CacheEntry *cacheEntry = getCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->reception : NULL;
}

void RadioChannel::setCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalReception *reception) const
{
    CacheEntry *cacheEntry = getCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->reception = reception;
}

void RadioChannel::removeCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    CacheEntry *cacheEntry = getCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->reception = NULL;
}

const IRadioSignalReceptionDecision *RadioChannel::getCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    CacheEntry *cacheEntry = getCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->decision : NULL;
}

void RadioChannel::setCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalReceptionDecision *decision) const
{
    CacheEntry *cacheEntry = getCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->decision = decision;
}

void RadioChannel::removeCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission) const
{
    CacheEntry *cacheEntry = getCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->decision = NULL;
}

void RadioChannel::invalidateCachedDecisions(const IRadioSignalTransmission *transmission)
{
    for (std::vector<std::vector<CacheEntry> *>::iterator it = cache.begin(); it != cache.end(); it++)
    {
        std::vector<CacheEntry> *cacheEntries = *it;
        if (cacheEntries)
        {
            for (std::vector<CacheEntry>::iterator jt = cacheEntries->begin(); jt != cacheEntries->end(); jt++)
            {
                CacheEntry &cacheEntry = *jt;
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
    CacheEntry *cacheEntry = getCacheEntry(radio, transmission);
    if (cacheEntry) cacheEntry->decision = NULL;
}

m RadioChannel::computeMaxRange(W maxTransmissionPower, W minReceptionPower) const
{
    double alpha = par("alpha");
    Hz carrierFrequency = Hz(par("carrierFrequency"));
    m waveLength = mps(SPEED_OF_LIGHT) / carrierFrequency;
    double maxAntennaGain = computeMaxAntennaGain();
    double minFactor = (minReceptionPower / maxAntennaGain / maxTransmissionPower).get();
    return waveLength / pow(minFactor * 16.0 * M_PI * M_PI, 1.0 / alpha);
}

m RadioChannel::computeMaxInterferenceRange() const
{
    m maxInterferenceRange = m(par("maxInterferenceRange"));
    Hz carrierFrequency = Hz(par("carrierFrequency"));
    if (!isNaN(maxInterferenceRange.get()))
        return maxInterferenceRange;
    else if (!isNaN(carrierFrequency.get()))
        return computeMaxRange(computeMaxTransmissionPower(), computeMinInterferencePower());
    else
        return m(qNaN);
}

m RadioChannel::computeMaxCommunicationRange() const
{
    m maxCommunicationRange = m(par("maxCommunicationRange"));
    Hz carrierFrequency = Hz(par("carrierFrequency"));
    if (!isNaN(maxCommunicationRange.get()))
        return maxCommunicationRange;
    else if (!isNaN(carrierFrequency.get()))
        return computeMaxRange(computeMaxTransmissionPower(), computeMinReceptionPower());
    else
        return m(qNaN);
}

W RadioChannel::computeMaxTransmissionPower() const
{
    // TODO: query all radios if unspecified?
    return W(par("maxTransmissionPower"));
}

W RadioChannel::computeMinInterferencePower() const
{
    // TODO: query all radios if unspecified?
    return mW(FWMath::dBm2mW(par("minInterferencePower")));
}

W RadioChannel::computeMinReceptionPower() const
{
    // TODO: query all radios if unspecified?
    return mW(FWMath::dBm2mW(par("minReceptionPower")));
}

double RadioChannel::computeMaxAntennaGain() const
{
    // TODO: query all radios if unspecified?
    return FWMath::dB2fraction(par("maxAntennaGain"));
}

const simtime_t RadioChannel::computeMinInterferenceTime() const
{
    return par("minInterferenceTime").doubleValue();
}

const simtime_t RadioChannel::computeMaxTransmissionDuration() const
{
    return par("maxTransmissionDuration").doubleValue();
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
    // TODO: erase non interfering transmissions at the exact time they need to be erased
    double minX = DBL_MAX;
    double maxX = DBL_MIN;
    double minY = DBL_MAX;
    double maxY = DBL_MIN;
    double minZ = DBL_MAX;
    double maxZ = DBL_MIN;
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
    {
        const IRadio *radio = *it;
        IMobility *mobility = radio->getAntenna()->getMobility();
        Coord position = mobility->getCurrentPosition();
        if (position.x < minX)
            minX = position.x;
        if (position.x > maxX)
            maxX = position.x;
        if (position.y < minY)
            minY = position.y;
        if (position.y > maxY)
            maxY = position.y;
        if (position.z < minZ)
            minZ = position.z;
        if (position.z > maxZ)
            maxZ = position.z;
    }
    double distanceMax = Coord(minX, minY, minZ).distance(Coord(maxX, maxY, maxZ));
    double maxPropagationTime = distanceMax / propagation->getPropagationSpeed().get();
    simtime_t minInterferingTransmissionEndTime = simTime() - maxPropagationTime - maxTransmissionDuration;
    if (minInterferingTransmissionEndTime >= 0)
    {
        EV_DEBUG << "Removing non-interfering transmissions that end before " << minInterferingTransmissionEndTime << " s" << endl;
        for (std::vector<const IRadioSignalTransmission *>::iterator it = transmissions.begin(); it != transmissions.end();)
        {
            const IRadioSignalTransmission *transmission = *it;
            if (transmission->getEndTime() < minInterferingTransmissionEndTime) {
                EV_DEBUG << "Removing non-interfering " << transmission << " from " << this << endl;
                const IRadioSignalTransmission *transmission = *it;
                // TODO: set to NULL only and remove them later along with cache entries
                transmissions.erase(it);
                removeNonInterferingTransmission(transmission);
                // TODO: revive after fingerprint kuldges are removed from radio
                // delete transmission;
                int transmissionIndex = transmission->getId() - baseTransmissionId;
                const std::vector<CacheEntry> *cacheEntries = cache[transmissionIndex];
                for (std::vector<CacheEntry>::const_iterator jt = cacheEntries->begin(); jt != cacheEntries->end(); jt++)
                {
                    const CacheEntry &cacheEntry = *jt;
                    delete cacheEntry.arrival;
                    delete cacheEntry.reception;
                    delete cacheEntry.decision;
                }
                delete cacheEntries;
                cache[transmissionIndex] = NULL;
            }
            else
                it++;
        }
        int transmissionIndex = 0;
        while (transmissionIndex < (int)cache.size() && !cache[transmissionIndex])
            transmissionIndex++;
        cache.erase(cache.begin(), cache.begin() + transmissionIndex);
        baseTransmissionId += transmissionIndex;
    }
    lastRemoveNonInterferingTransmissions = simTime();
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
    // TODO: add arrivals
}

void RadioChannel::removeRadio(const IRadio *radio)
{
    radios.erase(std::remove(radios.begin(), radios.end(), radio));
    // TODO: remove transmissions, arrivals
}

void RadioChannel::transmitToChannel(const IRadio *transmitterRadio, const IRadioSignalTransmission *transmission)
{
    transmissionCount++;
    transmissions.push_back(transmission);
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
    {
        const IRadio *receiverRadio = *it;
        if (receiverRadio != transmitterRadio)
        {
            const IRadioSignalArrival *arrival = propagation->computeArrival(transmission, receiverRadio->getAntenna()->getMobility());
            setCachedArrival(receiverRadio, transmission, arrival);
        }
    }
    // TODO: revise
    if (simTime() - lastRemoveNonInterferingTransmissions > maxTransmissionDuration)
        removeNonInterferingTransmissions();
}

void RadioChannel::sendToChannel(IRadio *radio, const IRadioFrame *frame)
{
    const Radio *transmitterRadio = check_and_cast<Radio *>(radio);
    const RadioFrame *radioFrame = check_and_cast<const RadioFrame *>(frame);
    const IRadioSignalTransmission *transmission = frame->getTransmission();
    if (recordCommunication)
        communicationLog << "T " << transmitterRadio->getFullPath() << " " << transmission->getTransmitter()->getId() << " "
                         << "M " << radioFrame->getName() << " " << transmission->getId() << " "
                         << "S " << transmission->getStartTime() << " " <<  transmission->getStartPosition() << " -> "
                         << "D " << transmission->getEndTime() << " " <<  transmission->getEndPosition() << endl;
    EV_DEBUG << "Sending " << frame << " with " << radioFrame->getBitLength() << " bits in " << radioFrame->getDuration() * 1E+6 << " us transmission duration"
             << " from " << radio << " on " << this << "." << endl;
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
    {
        const Radio *receiverRadio = check_and_cast<const Radio *>(*it);
        if (receiverRadio != transmitterRadio && isPotentialReceiver(receiverRadio, transmission))
        {
            cGate *gate = receiverRadio->RadioBase::getRadioGate()->getPathStartGate();
            const IRadioSignalArrival *arrival = getArrival(receiverRadio, transmission);
            simtime_t propagationTime = arrival->getStartPropagationTime();
            EV_DEBUG << "Sending " << frame
                     << " from " << radio << " at " << transmission->getStartPosition()
                     << " to " << *it << " at " << arrival->getStartPosition()
                     << " in " << propagationTime * 1E+6 << " us propagation time." << endl;
            RadioFrame *frameCopy = new RadioFrame(radioFrame->getTransmission());
            frameCopy->setName(radioFrame->getName());
            frameCopy->setDuration(radioFrame->getDuration());
            frameCopy->encapsulate(radioFrame->getEncapsulatedPacket()->dup());
            const_cast<Radio *>(transmitterRadio)->sendDirect(frameCopy, propagationTime, radioFrame->getDuration(), gate);
        }
    }
}

IRadioFrame *RadioChannel::transmitPacket(const IRadio *radio, cPacket *macFrame, const simtime_t startTime)
{
    const IRadioSignalTransmission *transmission = radio->getTransmitter()->createTransmission(radio, macFrame, startTime);
    transmitToChannel(radio, transmission);
    RadioFrame *radioFrame = new RadioFrame(transmission);
    radioFrame->setName(macFrame->getName());
    radioFrame->setDuration(transmission->getDuration());
    radioFrame->encapsulate(macFrame);
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
    return true;
    // TODO: KLUDGE: move to some scalar specific class
    // TODO: oversimplification
//    const ScalarRadioSignalReceiver *scalarReceiver = dynamic_cast<const ScalarRadioSignalReceiver *>(radio->getReceiver());
//    const ScalarRadioSignalTransmission *scalarTransmission = dynamic_cast<const ScalarRadioSignalTransmission *>(transmission);
//    if (scalarReceiver && scalarTransmission && scalarTransmission->getCarrierFrequency() != scalarReceiver->getCarrierFrequency())
//        return false;
//    else
//    {
//        const IRadioSignalArrival *arrival = getArrival(radio, transmission);
//        return isInCommunicationRange(transmission, arrival->getStartPosition(), arrival->getEndPosition());
//    }
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
