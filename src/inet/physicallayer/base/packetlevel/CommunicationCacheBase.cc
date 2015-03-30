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

#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/base/packetlevel/CommunicationCacheBase.h"

namespace inet {

namespace physicallayer {

CommunicationCacheBase::RadioCacheEntry::RadioCacheEntry() :
    receptionIntervals(nullptr)
{
}

CommunicationCacheBase::RadioCacheEntry::RadioCacheEntry(RadioCacheEntry &&other) :
    receptionIntervals(other.receptionIntervals)
{
    other.receptionIntervals = nullptr;
}

CommunicationCacheBase::RadioCacheEntry &CommunicationCacheBase::RadioCacheEntry::operator=(RadioCacheEntry &&other)
{
    if (this != &other) {
        delete receptionIntervals;
        receptionIntervals = other.receptionIntervals;
        other.receptionIntervals = nullptr;
    }
    return *this;
}

CommunicationCacheBase::RadioCacheEntry::~RadioCacheEntry()
{
    delete receptionIntervals;
}

CommunicationCacheBase::TransmissionCacheEntry::TransmissionCacheEntry() :
    interferenceEndTime(NaN),
    frame(nullptr),
    figure(nullptr),
    receptionCacheEntries(nullptr)
{
}

CommunicationCacheBase::ReceptionCacheEntry::ReceptionCacheEntry() :
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

CommunicationCacheBase::ReceptionCacheEntry::ReceptionCacheEntry(ReceptionCacheEntry &&other) :
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

CommunicationCacheBase::ReceptionCacheEntry &CommunicationCacheBase::ReceptionCacheEntry::operator=(ReceptionCacheEntry &&other)
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

CommunicationCacheBase::ReceptionCacheEntry::~ReceptionCacheEntry()
{
    delete arrival;
    delete listening;
    delete reception;
    delete interference;
    delete noise;
    delete snir;
    delete decision;
}

CommunicationCacheBase::CommunicationCacheBase()
{
}

CommunicationCacheBase::~CommunicationCacheBase()
{
}

std::vector<const ITransmission *> *CommunicationCacheBase::computeInterferingTransmissions(const IRadio *radio, const simtime_t startTime, const simtime_t endTime)
{
    RadioCacheEntry *radioCacheEntry = getRadioCacheEntry(radio);
    std::vector<const ITransmission *> *interferingTransmissions = new std::vector<const ITransmission *>();
    if (radioCacheEntry->receptionIntervals != nullptr) {
        std::deque<const Interval *> interferingIntervals = radioCacheEntry->receptionIntervals->query(startTime, endTime);
        for (auto interferingInterval : interferingIntervals) {
            const ITransmission *interferingTransmission = (ITransmission *)interferingInterval->value;
            interferingTransmissions->push_back(interferingTransmission);
        }
    }
    return interferingTransmissions;
}

const simtime_t CommunicationCacheBase::getCachedInterferenceEndTime(const ITransmission *transmission)
{
    return getTransmissionCacheEntry(transmission)->interferenceEndTime;
}

void CommunicationCacheBase::setCachedInterferenceEndTime(const ITransmission *transmission, const simtime_t interferenceEndTime)
{
    getTransmissionCacheEntry(transmission)->interferenceEndTime = interferenceEndTime;
}

void CommunicationCacheBase::removeCachedInterferenceEndTime(const ITransmission *transmission)
{
    getTransmissionCacheEntry(transmission)->interferenceEndTime = -1;
}

const IRadioFrame *CommunicationCacheBase::getCachedFrame(const ITransmission *transmission)
{
    return getTransmissionCacheEntry(transmission)->frame;
}

void CommunicationCacheBase::setCachedFrame(const ITransmission *transmission, const IRadioFrame *frame)
{
    getTransmissionCacheEntry(transmission)->frame = frame;
}

void CommunicationCacheBase::removeCachedFrame(const ITransmission *transmission)
{
    getTransmissionCacheEntry(transmission)->frame = nullptr;
}

cFigure *CommunicationCacheBase::getCachedFigure(const ITransmission *transmission)
{
    return getTransmissionCacheEntry(transmission)->figure;
}

void CommunicationCacheBase::setCachedFigure(const ITransmission *transmission, cFigure *figure)
{
    getTransmissionCacheEntry(transmission)->figure = figure;
}

void CommunicationCacheBase::removeCachedFigure(const ITransmission *transmission)
{
    getTransmissionCacheEntry(transmission)->figure = nullptr;
}

const IArrival *CommunicationCacheBase::getCachedArrival(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->arrival : nullptr;
}

void CommunicationCacheBase::setCachedArrival(const IRadio *radio, const ITransmission *transmission, const IArrival *arrival)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->arrival = arrival;
}

void CommunicationCacheBase::removeCachedArrival(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->arrival = nullptr;
}

const Interval *CommunicationCacheBase::getCachedInterval(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->interval : nullptr;
}

void CommunicationCacheBase::setCachedInterval(const IRadio *radio, const ITransmission *transmission, const Interval *interval)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->interval = interval;
    RadioCacheEntry *radioCacheEntry = getRadioCacheEntry(radio);
    if (radioCacheEntry->receptionIntervals == nullptr)
        radioCacheEntry->receptionIntervals = new IntervalTree();
    radioCacheEntry->receptionIntervals->insert(interval);
}

void CommunicationCacheBase::removeCachedInterval(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->interval = nullptr;
}

const IListening *CommunicationCacheBase::getCachedListening(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->listening : nullptr;
}

void CommunicationCacheBase::setCachedListening(const IRadio *radio, const ITransmission *transmission, const IListening *listening)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->listening = listening;
}

void CommunicationCacheBase::removeCachedListening(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->listening = nullptr;
}

const IReception *CommunicationCacheBase::getCachedReception(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->reception : nullptr;
}

void CommunicationCacheBase::setCachedReception(const IRadio *radio, const ITransmission *transmission, const IReception *reception)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->reception = reception;
}

void CommunicationCacheBase::removeCachedReception(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->reception = nullptr;
}

const IInterference *CommunicationCacheBase::getCachedInterference(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->interference : nullptr;
}

void CommunicationCacheBase::setCachedInterference(const IRadio *radio, const ITransmission *transmission, const IInterference *interference)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->interference = interference;
}

void CommunicationCacheBase::removeCachedInterference(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->interference = nullptr;
}

const INoise *CommunicationCacheBase::getCachedNoise(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->noise : nullptr;
}

void CommunicationCacheBase::setCachedNoise(const IRadio *radio, const ITransmission *transmission, const INoise *noise)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->noise = noise;
}

void CommunicationCacheBase::removeCachedNoise(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->noise = nullptr;
}

const ISNIR *CommunicationCacheBase::getCachedSNIR(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->snir : nullptr;
}

void CommunicationCacheBase::setCachedSNIR(const IRadio *radio, const ITransmission *transmission, const ISNIR *snir)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->snir = snir;
}

void CommunicationCacheBase::removeCachedSNIR(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->snir = nullptr;
}

const IReceptionDecision *CommunicationCacheBase::getCachedDecision(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->decision : nullptr;
}

void CommunicationCacheBase::setCachedDecision(const IRadio *radio, const ITransmission *transmission, const IReceptionDecision *decision)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->decision = decision;
}

void CommunicationCacheBase::removeCachedDecision(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->decision = nullptr;
}

const IRadioFrame *CommunicationCacheBase::getCachedFrame(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    return cacheEntry ? cacheEntry->frame : nullptr;
}

void CommunicationCacheBase::setCachedFrame(const IRadio *radio, const ITransmission *transmission, const IRadioFrame *frame)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->frame = frame;
}

void CommunicationCacheBase::removeCachedFrame(const IRadio *radio, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(radio, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->frame = nullptr;
}

} // namespace physicallayer

} // namespace inet

