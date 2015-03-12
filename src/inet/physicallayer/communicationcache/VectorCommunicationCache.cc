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

#include "inet/physicallayer/contract/IRadio.h"
#include "inet/physicallayer/communicationcache/VectorCommunicationCache.h"

namespace inet {

namespace physicallayer {

Define_Module(VectorCommunicationCache);

VectorCommunicationCache::RadioCacheEntry::RadioCacheEntry() :
    receptionIntervals(nullptr)
{
}

VectorCommunicationCache::TransmissionCacheEntry::TransmissionCacheEntry() :
    interferenceEndTime(NaN),
    frame(nullptr),
    figure(nullptr),
    receptionCacheEntries(nullptr)
{
}

VectorCommunicationCache::ReceptionCacheEntry::ReceptionCacheEntry() :
    frame(nullptr),
    arrival(nullptr),
    interval(nullptr),
    listening(nullptr),
    reception(nullptr),
    interference(nullptr),
    noise(nullptr),
    snir(nullptr),
    decision(nullptr)
{
}

VectorCommunicationCache::ReceptionCacheEntry::ReceptionCacheEntry(ReceptionCacheEntry &&other) :
    frame(other.frame),
    arrival(other.arrival),
    interval(other.interval),
    listening(other.listening),
    reception(other.reception),
    interference(other.interference),
    noise(other.noise),
    snir(other.snir),
    decision(other.decision)
{
    other.frame = nullptr;
    other.arrival = nullptr;
    other.interval = nullptr;
    other.listening = nullptr;
    other.reception = nullptr;
    other.interference = nullptr;
    other.noise = nullptr;
    other.snir = nullptr;
    other.decision = nullptr;
}

VectorCommunicationCache::ReceptionCacheEntry &VectorCommunicationCache::ReceptionCacheEntry::operator=(ReceptionCacheEntry &&other)
{
    if (this != &other) {
        delete arrival;
        delete listening;
        delete reception;
        delete interference;
        delete noise;
        delete snir;
        delete decision;

        frame = other.frame;
        arrival = other.arrival;
        interval = other.interval;
        listening = other.listening;
        reception = other.reception;
        interference = other.interference;
        noise = other.noise;
        snir = other.snir;
        decision = other.decision;

        other.frame = nullptr;
        other.arrival = nullptr;
        other.interval  = nullptr;
        other.listening = nullptr;
        other.reception = nullptr;
        other.interference = nullptr;
        other.noise = nullptr;
        other.snir = nullptr;
        other.decision = nullptr;
    }
    return *this;
}

VectorCommunicationCache::ReceptionCacheEntry::~ReceptionCacheEntry()
{
    delete arrival;
    delete listening;
    delete reception;
    delete interference;
    delete noise;
    delete snir;
    delete decision;
}

VectorCommunicationCache::VectorCommunicationCache() :
    baseRadioId(0),
    baseTransmissionId(0)
{
}

VectorCommunicationCache::~VectorCommunicationCache()
{
    for (auto &transmissionCacheEntry : transmissionCache) {
        delete transmissionCacheEntry.frame;
        delete transmissionCacheEntry.receptionCacheEntries;
    }
    for (auto &radioCacheEntry : radioCache)
        delete radioCacheEntry.receptionIntervals;
}

VectorCommunicationCache::RadioCacheEntry *VectorCommunicationCache::getRadioCacheEntry(const IRadio *radio)
{
    int radioIndex = radio->getId() - baseRadioId;
    if (radioIndex < 0)
        return nullptr;
    else {
        if (radioIndex >= (int)radioCache.size())
            radioCache.resize(radioIndex + 1);
        RadioCacheEntry *radioCacheEntry = &radioCache[radioIndex];
        if (!radioCacheEntry->receptionIntervals)
            radioCacheEntry->receptionIntervals = new IntervalTree();
        return radioCacheEntry;
    }
}

VectorCommunicationCache::TransmissionCacheEntry *VectorCommunicationCache::getTransmissionCacheEntry(const ITransmission *transmission)
{
    int transmissionIndex = transmission->getId() - baseTransmissionId;
    if (transmissionIndex < 0)
        return nullptr;
    else {
        if (transmissionIndex >= (int)transmissionCache.size())
            transmissionCache.resize(transmissionIndex + 1);
        TransmissionCacheEntry &transmissionCacheEntry = transmissionCache[transmissionIndex];
        if (!transmissionCacheEntry.receptionCacheEntries)
            transmissionCacheEntry.receptionCacheEntries = new std::vector<ReceptionCacheEntry>(radioCache.size());
        return &transmissionCacheEntry;
    }
}

VectorCommunicationCache::ReceptionCacheEntry *VectorCommunicationCache::getReceptionCacheEntry(const IRadio *radio, const ITransmission *transmission)
{
    TransmissionCacheEntry *transmissionCacheEntry = getTransmissionCacheEntry(transmission);
    if (!transmissionCacheEntry)
        return nullptr;
    else {
        std::vector<ReceptionCacheEntry> *receptionCacheEntries = transmissionCacheEntry->receptionCacheEntries;
        int radioIndex = radio->getId() - baseRadioId;
        if (radioIndex < 0)
            return nullptr;
        else {
            if (radioIndex >= (int)receptionCacheEntries->size())
                receptionCacheEntries->resize(radioIndex + 1);
            return &(*receptionCacheEntries)[radioIndex];
        }
    }
}

void VectorCommunicationCache::addRadio(const IRadio *radio)
{
    if (radioCache.size() == 0)
        baseRadioId = radio->getId();
    getRadioCacheEntry(radio);
}

void VectorCommunicationCache::removeRadio(const IRadio *radio)
{
    int radioCount = 0;
    while (radioCache[radioCount].receptionIntervals == nullptr && radioCount < (int)radioCache.size())
        radioCount++;
    if (radioCount != 0) {
        baseRadioId += radioCount;
        radioCache.erase(radioCache.begin(), radioCache.begin() + radioCount);
        for (auto &transmissionCacheEntry : transmissionCache) {
            std::vector<ReceptionCacheEntry> *receptionCacheEntries = transmissionCacheEntry.receptionCacheEntries;
            if (receptionCacheEntries != nullptr)
                receptionCacheEntries->erase(receptionCacheEntries->begin(), receptionCacheEntries->begin() + radioCount);
        }
    }
}

void VectorCommunicationCache::addTransmission(const ITransmission *transmission)
{
    if (transmissionCache.size() == 0)
        baseTransmissionId = transmission->getId();
    getTransmissionCacheEntry(transmission);
}

void VectorCommunicationCache::removeTransmission(const ITransmission *transmission)
{
}

std::vector<const ITransmission *> *VectorCommunicationCache::computeInterferingTransmissions(const IRadio *radio, const simtime_t startTime, const simtime_t endTime)
{
    RadioCacheEntry *radioCacheEntry = getRadioCacheEntry(radio);
    std::deque<const Interval *> interferingIntervals = radioCacheEntry->receptionIntervals->query(startTime, endTime);
    std::vector<const ITransmission *> *interferingTransmissions = new std::vector<const ITransmission *>();
    for (auto interferingInterval : interferingIntervals) {
        const ITransmission *interferingTransmission = (ITransmission *)interferingInterval->value;
        interferingTransmissions->push_back(interferingTransmission);
    }
    return interferingTransmissions;
}

void VectorCommunicationCache::removeNonInterferingTransmissions()
{
    const simtime_t now = simTime();
    size_t transmissionIndex = 0;
    while (transmissionIndex < transmissionCache.size() && transmissionCache[transmissionIndex].interferenceEndTime <= now)
        transmissionIndex++;
    for (auto it = transmissionCache.cbegin(); it != transmissionCache.cbegin() + transmissionIndex; it++) {
        const TransmissionCacheEntry &transmissionCacheEntry = *it;
        if (transmissionCacheEntry.receptionCacheEntries != nullptr) {
            auto radioIt = radioCache.cbegin();
            auto receptionIt = transmissionCacheEntry.receptionCacheEntries->cbegin();
            while (radioIt != radioCache.cend() && receptionIt != transmissionCacheEntry.receptionCacheEntries->cend()) {
                const RadioCacheEntry &radioCacheEntry = *radioIt;
                const ReceptionCacheEntry &recpeionCacheEntry = *receptionIt;
                const Interval *interval = recpeionCacheEntry.interval;
                if (interval != nullptr)
                    radioCacheEntry.receptionIntervals->deleteNode(interval);
                radioIt++;
                receptionIt++;
            }
        }
    }
    baseTransmissionId += transmissionIndex;
    for (auto it = transmissionCache.cbegin(); it != transmissionCache.cbegin() + transmissionIndex; it++) {
        const TransmissionCacheEntry &transmissionCacheEntry = *it;
        delete transmissionCacheEntry.frame;
        delete transmissionCacheEntry.receptionCacheEntries;
    }
    transmissionCache.erase(transmissionCache.begin(), transmissionCache.begin() + transmissionIndex);
}

const simtime_t VectorCommunicationCache::getCachedInterferenceEndTime(const ITransmission *transmission)
{
    return getTransmissionCacheEntry(transmission)->interferenceEndTime;
}

void VectorCommunicationCache::setCachedInterferenceEndTime(const ITransmission *transmission, const simtime_t interferenceEndTime)
{
    getTransmissionCacheEntry(transmission)->interferenceEndTime = interferenceEndTime;
}

void VectorCommunicationCache::removeCachedInterferenceEndTime(const ITransmission *transmission)
{
    getTransmissionCacheEntry(transmission)->interferenceEndTime = -1;
}

const IRadioFrame *VectorCommunicationCache::getCachedFrame(const ITransmission *transmission)
{
    return getTransmissionCacheEntry(transmission)->frame;
}

void VectorCommunicationCache::setCachedFrame(const ITransmission *transmission, const IRadioFrame *frame)
{
    getTransmissionCacheEntry(transmission)->frame = frame;
}

void VectorCommunicationCache::removeCachedFrame(const ITransmission *transmission)
{
    getTransmissionCacheEntry(transmission)->frame = nullptr;
}

cFigure *VectorCommunicationCache::getCachedFigure(const ITransmission *transmission)
{
    return getTransmissionCacheEntry(transmission)->figure;
}

void VectorCommunicationCache::setCachedFigure(const ITransmission *transmission, cFigure *figure)
{
    getTransmissionCacheEntry(transmission)->figure = figure;
}

void VectorCommunicationCache::removeCachedFigure(const ITransmission *transmission)
{
    getTransmissionCacheEntry(transmission)->figure = nullptr;
}

const IArrival *VectorCommunicationCache::getCachedArrival(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->arrival : nullptr;
}

void VectorCommunicationCache::setCachedArrival(const IRadio *radio, const ITransmission *transmission, const IArrival *arrival)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->arrival = arrival;
}

void VectorCommunicationCache::removeCachedArrival(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->arrival = nullptr;
}

const Interval *VectorCommunicationCache::getCachedInterval(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->interval : nullptr;
}

void VectorCommunicationCache::setCachedInterval(const IRadio *radio, const ITransmission *transmission, const Interval *interval)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->interval = interval;
    RadioCacheEntry *radioCacheEntry = getRadioCacheEntry(radio);
    radioCacheEntry->receptionIntervals->insert(interval);
}

void VectorCommunicationCache::removeCachedInterval(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->interval = nullptr;
}

const IListening *VectorCommunicationCache::getCachedListening(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->listening : nullptr;
}

void VectorCommunicationCache::setCachedListening(const IRadio *radio, const ITransmission *transmission, const IListening *listening)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->listening = listening;
}

void VectorCommunicationCache::removeCachedListening(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->listening = nullptr;
}

const IReception *VectorCommunicationCache::getCachedReception(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->reception : nullptr;
}

void VectorCommunicationCache::setCachedReception(const IRadio *radio, const ITransmission *transmission, const IReception *reception)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->reception = reception;
}

void VectorCommunicationCache::removeCachedReception(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->reception = nullptr;
}

const IInterference *VectorCommunicationCache::getCachedInterference(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->interference : nullptr;
}

void VectorCommunicationCache::setCachedInterference(const IRadio *radio, const ITransmission *transmission, const IInterference *interference)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->interference = interference;
}

void VectorCommunicationCache::removeCachedInterference(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->interference = nullptr;
}

const INoise *VectorCommunicationCache::getCachedNoise(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->noise : nullptr;
}

void VectorCommunicationCache::setCachedNoise(const IRadio *radio, const ITransmission *transmission, const INoise *noise)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->noise = noise;
}

void VectorCommunicationCache::removeCachedNoise(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->noise = nullptr;
}

const ISNIR *VectorCommunicationCache::getCachedSNIR(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->snir : nullptr;
}

void VectorCommunicationCache::setCachedSNIR(const IRadio *radio, const ITransmission *transmission, const ISNIR *snir)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->snir = snir;
}

void VectorCommunicationCache::removeCachedSNIR(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->snir = nullptr;
}

const IReceptionDecision *VectorCommunicationCache::getCachedDecision(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->decision : nullptr;
}

void VectorCommunicationCache::setCachedDecision(const IRadio *radio, const ITransmission *transmission, const IReceptionDecision *decision)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->decision = decision;
}

void VectorCommunicationCache::removeCachedDecision(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->decision = nullptr;
}

const IRadioFrame *VectorCommunicationCache::getCachedFrame(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->frame : nullptr;
}

void VectorCommunicationCache::setCachedFrame(const IRadio *radio, const ITransmission *transmission, const IRadioFrame *frame)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->frame = frame;
}

void VectorCommunicationCache::removeCachedFrame(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry)
        cacheEntry->frame = nullptr;
}

} // namespace physicallayer

} // namespace inet

