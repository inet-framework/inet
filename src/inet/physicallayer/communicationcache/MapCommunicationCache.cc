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
#include "inet/physicallayer/communicationcache/MapCommunicationCache.h"

namespace inet {

namespace physicallayer {

Define_Module(MapCommunicationCache);

MapCommunicationCache::RadioCacheEntry::RadioCacheEntry() :
    receptionIntervals(nullptr)
{
}

MapCommunicationCache::TransmissionCacheEntry::TransmissionCacheEntry() :
    interferenceEndTime(NaN),
    frame(nullptr),
    figure(nullptr),
    receptionCacheEntries(nullptr)
{
}

MapCommunicationCache::ReceptionCacheEntry::ReceptionCacheEntry() :
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

MapCommunicationCache::ReceptionCacheEntry::ReceptionCacheEntry(ReceptionCacheEntry &&other) :
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

MapCommunicationCache::ReceptionCacheEntry &MapCommunicationCache::ReceptionCacheEntry::operator=(ReceptionCacheEntry &&other)
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

MapCommunicationCache::ReceptionCacheEntry::~ReceptionCacheEntry()
{
    delete arrival;
    delete listening;
    delete reception;
    delete interference;
    delete noise;
    delete snir;
    delete decision;
}

MapCommunicationCache::MapCommunicationCache()
{
}

MapCommunicationCache::~MapCommunicationCache()
{
    for (auto &transmissionCacheEntry : transmissionCache) {
        delete transmissionCacheEntry.second.frame;
        delete transmissionCacheEntry.second.receptionCacheEntries;
    }
    for (auto &radioCacheEntry : radioCache)
        delete radioCacheEntry.second.receptionIntervals;
}

MapCommunicationCache::RadioCacheEntry *MapCommunicationCache::getRadioCacheEntry(const IRadio *radio)
{
    return &radioCache[radio];
}

MapCommunicationCache::TransmissionCacheEntry *MapCommunicationCache::getTransmissionCacheEntry(const ITransmission *transmission)
{
    return &transmissionCache[transmission];
}

MapCommunicationCache::ReceptionCacheEntry *MapCommunicationCache::getReceptionCacheEntry(const IRadio *radio, const ITransmission *transmission)
{
    TransmissionCacheEntry *transmissionCacheEntry = getTransmissionCacheEntry(transmission);
    if (transmissionCacheEntry == nullptr)
        return nullptr;
    else
        return &(*transmissionCacheEntry->receptionCacheEntries)[radio];
}

void MapCommunicationCache::addRadio(const IRadio *radio)
{
}

void MapCommunicationCache::removeRadio(const IRadio *radio)
{
    radioCache.erase(radioCache.find(radio));
    for (auto &transmissionCacheEntry : transmissionCache) {
        std::map<const IRadio *, ReceptionCacheEntry> *receptionCacheEntries = transmissionCacheEntry.second.receptionCacheEntries;
        if (receptionCacheEntries != nullptr)
            receptionCacheEntries->erase(receptionCacheEntries->find(radio));
    }
}

void MapCommunicationCache::addTransmission(const ITransmission *transmission)
{
}

void MapCommunicationCache::removeTransmission(const ITransmission *transmission)
{
    auto it = transmissionCache.find(transmission);
    const TransmissionCacheEntry &transmissionCacheEntry = (*it).second;
    if (transmissionCacheEntry.receptionCacheEntries != nullptr) {
        auto receptionIt = transmissionCacheEntry.receptionCacheEntries->cbegin();
        while (receptionIt != transmissionCacheEntry.receptionCacheEntries->cend()) {
            const IRadio *radio = (*receptionIt).first;
            const RadioCacheEntry &radioCacheEntry = radioCache[radio];
            const ReceptionCacheEntry &recpeionCacheEntry = (*receptionIt).second;
            const Interval *interval = recpeionCacheEntry.interval;
            if (interval != nullptr)
                radioCacheEntry.receptionIntervals->deleteNode(interval);
            receptionIt++;
        }
    }
    delete transmissionCacheEntry.frame;
    delete transmissionCacheEntry.receptionCacheEntries;
    transmissionCache.erase(it);
}

std::vector<const ITransmission *> *MapCommunicationCache::computeInterferingTransmissions(const IRadio *radio, const simtime_t startTime, const simtime_t endTime)
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

void MapCommunicationCache::removeNonInterferingTransmissions()
{
}

const simtime_t MapCommunicationCache::getCachedInterferenceEndTime(const ITransmission *transmission)
{
    return getTransmissionCacheEntry(transmission)->interferenceEndTime;
}

void MapCommunicationCache::setCachedInterferenceEndTime(const ITransmission *transmission, const simtime_t interferenceEndTime)
{
    getTransmissionCacheEntry(transmission)->interferenceEndTime = interferenceEndTime;
}

void MapCommunicationCache::removeCachedInterferenceEndTime(const ITransmission *transmission)
{
    getTransmissionCacheEntry(transmission)->interferenceEndTime = -1;
}

const IRadioFrame *MapCommunicationCache::getCachedFrame(const ITransmission *transmission)
{
    return getTransmissionCacheEntry(transmission)->frame;
}

void MapCommunicationCache::setCachedFrame(const ITransmission *transmission, const IRadioFrame *frame)
{
    getTransmissionCacheEntry(transmission)->frame = frame;
}

void MapCommunicationCache::removeCachedFrame(const ITransmission *transmission)
{
    getTransmissionCacheEntry(transmission)->frame = nullptr;
}

cFigure *MapCommunicationCache::getCachedFigure(const ITransmission *transmission)
{
    return getTransmissionCacheEntry(transmission)->figure;
}

void MapCommunicationCache::setCachedFigure(const ITransmission *transmission, cFigure *figure)
{
    getTransmissionCacheEntry(transmission)->figure = figure;
}

void MapCommunicationCache::removeCachedFigure(const ITransmission *transmission)
{
    getTransmissionCacheEntry(transmission)->figure = nullptr;
}

const IArrival *MapCommunicationCache::getCachedArrival(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->arrival : nullptr;
}

void MapCommunicationCache::setCachedArrival(const IRadio *radio, const ITransmission *transmission, const IArrival *arrival)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->arrival = arrival;
}

void MapCommunicationCache::removeCachedArrival(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->arrival = nullptr;
}

const Interval *MapCommunicationCache::getCachedInterval(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->interval : nullptr;
}

void MapCommunicationCache::setCachedInterval(const IRadio *radio, const ITransmission *transmission, const Interval *interval)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->interval = interval;
    RadioCacheEntry *radioCacheEntry = getRadioCacheEntry(radio);
    radioCacheEntry->receptionIntervals->insert(interval);
}

void MapCommunicationCache::removeCachedInterval(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->interval = nullptr;
}

const IListening *MapCommunicationCache::getCachedListening(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->listening : nullptr;
}

void MapCommunicationCache::setCachedListening(const IRadio *radio, const ITransmission *transmission, const IListening *listening)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->listening = listening;
}

void MapCommunicationCache::removeCachedListening(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->listening = nullptr;
}

const IReception *MapCommunicationCache::getCachedReception(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->reception : nullptr;
}

void MapCommunicationCache::setCachedReception(const IRadio *radio, const ITransmission *transmission, const IReception *reception)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->reception = reception;
}

void MapCommunicationCache::removeCachedReception(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->reception = nullptr;
}

const IInterference *MapCommunicationCache::getCachedInterference(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->interference : nullptr;
}

void MapCommunicationCache::setCachedInterference(const IRadio *radio, const ITransmission *transmission, const IInterference *interference)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->interference = interference;
}

void MapCommunicationCache::removeCachedInterference(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->interference = nullptr;
}

const INoise *MapCommunicationCache::getCachedNoise(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->noise : nullptr;
}

void MapCommunicationCache::setCachedNoise(const IRadio *radio, const ITransmission *transmission, const INoise *noise)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->noise = noise;
}

void MapCommunicationCache::removeCachedNoise(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->noise = nullptr;
}

const ISNIR *MapCommunicationCache::getCachedSNIR(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->snir : nullptr;
}

void MapCommunicationCache::setCachedSNIR(const IRadio *radio, const ITransmission *transmission, const ISNIR *snir)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->snir = snir;
}

void MapCommunicationCache::removeCachedSNIR(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->snir = nullptr;
}

const IReceptionDecision *MapCommunicationCache::getCachedDecision(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->decision : nullptr;
}

void MapCommunicationCache::setCachedDecision(const IRadio *radio, const ITransmission *transmission, const IReceptionDecision *decision)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->decision = decision;
}

void MapCommunicationCache::removeCachedDecision(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->decision = nullptr;
}

const IRadioFrame *MapCommunicationCache::getCachedFrame(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->frame : nullptr;
}

void MapCommunicationCache::setCachedFrame(const IRadio *radio, const ITransmission *transmission, const IRadioFrame *frame)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->frame = frame;
}

void MapCommunicationCache::removeCachedFrame(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->frame = nullptr;
}

} // namespace physicallayer

} // namespace inet

