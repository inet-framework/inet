//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/CommunicationCacheBase.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {

namespace physicallayer {

CommunicationCacheBase::RadioCacheEntry::RadioCacheEntry(const RadioCacheEntry& other) :
    radio(other.radio),
    receptionIntervals(other.receptionIntervals)
{
    // NOTE: only allow copying mostly empty ones for now
    ASSERT(other.receptionIntervals == nullptr);
}

CommunicationCacheBase::RadioCacheEntry::RadioCacheEntry(RadioCacheEntry&& other) noexcept :
    radio(other.radio),
    receptionIntervals(other.receptionIntervals)
{
    other.receptionIntervals = nullptr;
}

CommunicationCacheBase::RadioCacheEntry& CommunicationCacheBase::RadioCacheEntry::operator=(const RadioCacheEntry& other)
{
    if (this != &other) {
        // NOTE: only allow copying mostly empty ones for now
        ASSERT(other.receptionIntervals == nullptr);
        radio = other.radio;
        delete receptionIntervals;
        receptionIntervals = other.receptionIntervals;
    }
    return *this;
}

CommunicationCacheBase::RadioCacheEntry& CommunicationCacheBase::RadioCacheEntry::operator=(RadioCacheEntry&& other) noexcept
{
    if (this != &other) {
        radio = other.radio;
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

CommunicationCacheBase::ReceptionCacheEntry::ReceptionCacheEntry()
{
    receptionDecisions.resize(static_cast<int>(IRadioSignal::SIGNAL_PART_DATA) + 1);
}

CommunicationCacheBase::ReceptionCacheEntry::ReceptionCacheEntry(const ReceptionCacheEntry& other) :
    transmission(other.transmission),
    receiver(other.receiver),
    signal(other.signal),
    arrival(other.arrival),
    interval(other.interval),
    listening(other.listening),
    reception(other.reception),
    interference(other.interference),
    noise(other.noise),
    snir(other.snir),
    receptionDecisions(other.receptionDecisions),
    receptionResult(other.receptionResult)
{
    // NOTE: only allow copying empty mostly ones for now
    ASSERT(other.signal == nullptr);
    ASSERT(other.arrival == nullptr);
    ASSERT(other.interval == nullptr);
    ASSERT(other.listening == nullptr);
    ASSERT(other.reception == nullptr);
    ASSERT(other.interference == nullptr);
    ASSERT(other.noise == nullptr);
    ASSERT(other.snir == nullptr);
    ASSERT(other.receptionDecisions.size() == 0);
    ASSERT(other.receptionResult == nullptr);
}

CommunicationCacheBase::ReceptionCacheEntry::ReceptionCacheEntry(ReceptionCacheEntry&& other) noexcept :
    transmission(other.transmission),
    receiver(other.receiver),
    signal(other.signal),
    arrival(other.arrival),
    interval(other.interval),
    listening(other.listening),
    reception(other.reception),
    interference(other.interference),
    noise(other.noise),
    snir(other.snir),
    receptionDecisions(other.receptionDecisions),
    receptionResult(other.receptionResult)
{
    other.transmission = nullptr;
    other.receiver = nullptr;
    other.signal = nullptr;
    other.arrival = nullptr;
    other.interval = nullptr;
    other.listening = nullptr;
    other.reception = nullptr;
    other.interference = nullptr;
    other.noise = nullptr;
    other.snir = nullptr;
    other.receptionDecisions.clear();
    other.receptionResult = nullptr;
}

CommunicationCacheBase::ReceptionCacheEntry& CommunicationCacheBase::ReceptionCacheEntry::operator=(const ReceptionCacheEntry& other)
{
    if (this != &other) {
        // NOTE: only allow copying mostly empty ones for now
        ASSERT(other.signal == nullptr);
        ASSERT(other.arrival == nullptr);
        ASSERT(other.interval == nullptr);
        ASSERT(other.listening == nullptr);
        ASSERT(other.reception == nullptr);
        ASSERT(other.interference == nullptr);
        ASSERT(other.noise == nullptr);
        ASSERT(other.snir == nullptr);
        ASSERT(other.receptionDecisions.size() == 0);
        ASSERT(other.receptionResult == nullptr);
        transmission = other.transmission;
        receiver = other.receiver;
        signal = other.signal;
        arrival = other.arrival;
        interval = other.interval;
        listening = other.listening;
        reception = other.reception;
        interference = other.interference;
        noise = other.noise;
        snir = other.snir;
        for (auto receptionDecision : receptionDecisions)
            delete receptionDecision;
        receptionDecisions = other.receptionDecisions;
        receptionResult = other.receptionResult;
    }
    return *this;
}

CommunicationCacheBase::ReceptionCacheEntry& CommunicationCacheBase::ReceptionCacheEntry::operator=(ReceptionCacheEntry&& other) noexcept
{
    if (this != &other) {
        delete arrival;
        delete listening;
        delete reception;
        delete interference;
        delete noise;
        delete snir;
        for (auto receptionDecision : receptionDecisions)
            delete receptionDecision;
        delete receptionResult;
        transmission = other.transmission;
        receiver = other.receiver;
        signal = other.signal;
        arrival = other.arrival;
        interval = other.interval;
        listening = other.listening;
        reception = other.reception;
        interference = other.interference;
        noise = other.noise;
        snir = other.snir;
        receptionDecisions = other.receptionDecisions;
        receptionResult = other.receptionResult;
        other.transmission = nullptr;
        other.receiver = nullptr;
        other.signal = nullptr;
        other.arrival = nullptr;
        other.interval = nullptr;
        other.listening = nullptr;
        other.reception = nullptr;
        other.interference = nullptr;
        other.noise = nullptr;
        other.snir = nullptr;
        other.receptionDecisions.clear();
        other.receptionResult = nullptr;
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
    for (auto receptionDecision : receptionDecisions)
        delete receptionDecision;
    delete receptionResult;
}

CommunicationCacheBase::TransmissionCacheEntry::TransmissionCacheEntry(const TransmissionCacheEntry& other) :
    transmission(other.transmission),
    interferenceEndTime(other.interferenceEndTime),
    signal(other.signal)
{
}

CommunicationCacheBase::TransmissionCacheEntry::TransmissionCacheEntry(TransmissionCacheEntry&& other) noexcept :
    transmission(other.transmission),
    interferenceEndTime(other.interferenceEndTime),
    signal(other.signal)
{
    other.transmission = nullptr;
    other.interferenceEndTime = 0;
    other.signal = nullptr;
}

CommunicationCacheBase::TransmissionCacheEntry::~TransmissionCacheEntry()
{
    deleteSignal();
    delete transmission;
    transmission = nullptr;
}

CommunicationCacheBase::TransmissionCacheEntry& CommunicationCacheBase::TransmissionCacheEntry::operator=(const TransmissionCacheEntry& other)
{
    if (this != &other) {
        deleteSignal();
        delete transmission;
        transmission = other.transmission;
        interferenceEndTime = other.interferenceEndTime;
        signal = other.signal;
    }
    return *this;
}

CommunicationCacheBase::TransmissionCacheEntry& CommunicationCacheBase::TransmissionCacheEntry::operator=(TransmissionCacheEntry&& other) noexcept
{
    if (this != &other) {
        deleteSignal();
        delete transmission;
        transmission = other.transmission;
        interferenceEndTime = other.interferenceEndTime;
        signal = other.signal;
        other.transmission = nullptr;
        other.interferenceEndTime = 0;
        other.signal = nullptr;
    }
    return *this;
}

void CommunicationCacheBase::TransmissionCacheEntry::deleteSignal()
{
    if (signal != nullptr) {
        // NOTE: we pretend that the owning context is the packet owner in order to be able to delete it
        auto packet = check_and_cast<const cPacket *>(signal);
        cContextSwitcher contextSwitcher(check_and_cast<cComponent *>(packet->getOwner()));
        delete signal;
        signal = nullptr;
    }
}

int CommunicationCacheBase::getNumRadios() const
{
    int numRadios = 0;
    mapRadios([&](const IRadio *radio) { numRadios++; });
    return numRadios;
}

int CommunicationCacheBase::getNumTransmissions() const
{
    int numTransmissions = 0;
    mapTransmissions([&](const ITransmission *transmission) { numTransmissions++; });
    return numTransmissions;
}

std::vector<const ITransmission *> *CommunicationCacheBase::computeInterferingTransmissions(const IRadio *radio, const simtime_t startTime, const simtime_t endTime)
{
    RadioCacheEntry *radioCacheEntry = getRadioCacheEntry(radio);
    std::vector<const ITransmission *> *interferingTransmissions = new std::vector<const ITransmission *>();
    if (radioCacheEntry->receptionIntervals != nullptr) {
        std::deque<const IntervalTree::Interval *> interferingIntervals = radioCacheEntry->receptionIntervals->query(startTime, endTime);
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

const IWirelessSignal *CommunicationCacheBase::getCachedSignal(const ITransmission *transmission)
{
    return getTransmissionCacheEntry(transmission)->signal;
}

void CommunicationCacheBase::setCachedSignal(const ITransmission *transmission, const IWirelessSignal *signal)
{
    getTransmissionCacheEntry(transmission)->signal = signal;
}

void CommunicationCacheBase::removeCachedSignal(const ITransmission *transmission)
{
    getTransmissionCacheEntry(transmission)->signal = nullptr;
}

const IArrival *CommunicationCacheBase::getCachedArrival(const IRadio *receiver, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    return cacheEntry ? cacheEntry->arrival : nullptr;
}

void CommunicationCacheBase::setCachedArrival(const IRadio *receiver, const ITransmission *transmission, const IArrival *arrival)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->arrival = arrival;
}

void CommunicationCacheBase::removeCachedArrival(const IRadio *receiver, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->arrival = nullptr;
}

const IntervalTree::Interval *CommunicationCacheBase::getCachedInterval(const IRadio *receiver, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    return cacheEntry ? cacheEntry->interval : nullptr;
}

void CommunicationCacheBase::setCachedInterval(const IRadio *receiver, const ITransmission *transmission, const IntervalTree::Interval *interval)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->interval = interval;
    RadioCacheEntry *radioCacheEntry = getRadioCacheEntry(receiver);
    if (radioCacheEntry->receptionIntervals == nullptr)
        radioCacheEntry->receptionIntervals = new IntervalTree();
    radioCacheEntry->receptionIntervals->insert(interval);
}

void CommunicationCacheBase::removeCachedInterval(const IRadio *receiver, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->interval = nullptr;
}

const IListening *CommunicationCacheBase::getCachedListening(const IRadio *receiver, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    return cacheEntry ? cacheEntry->listening : nullptr;
}

void CommunicationCacheBase::setCachedListening(const IRadio *receiver, const ITransmission *transmission, const IListening *listening)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->listening = listening;
}

void CommunicationCacheBase::removeCachedListening(const IRadio *receiver, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->listening = nullptr;
}

const IReception *CommunicationCacheBase::getCachedReception(const IRadio *receiver, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    return cacheEntry ? cacheEntry->reception : nullptr;
}

void CommunicationCacheBase::setCachedReception(const IRadio *receiver, const ITransmission *transmission, const IReception *reception)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->reception = reception;
}

void CommunicationCacheBase::removeCachedReception(const IRadio *receiver, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->reception = nullptr;
}

const IInterference *CommunicationCacheBase::getCachedInterference(const IRadio *receiver, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    return cacheEntry ? cacheEntry->interference : nullptr;
}

void CommunicationCacheBase::setCachedInterference(const IRadio *receiver, const ITransmission *transmission, const IInterference *interference)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->interference = interference;
}

void CommunicationCacheBase::removeCachedInterference(const IRadio *receiver, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->interference = nullptr;
}

const INoise *CommunicationCacheBase::getCachedNoise(const IRadio *receiver, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    return cacheEntry ? cacheEntry->noise : nullptr;
}

void CommunicationCacheBase::setCachedNoise(const IRadio *receiver, const ITransmission *transmission, const INoise *noise)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->noise = noise;
}

void CommunicationCacheBase::removeCachedNoise(const IRadio *receiver, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->noise = nullptr;
}

const ISnir *CommunicationCacheBase::getCachedSNIR(const IRadio *receiver, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    return cacheEntry ? cacheEntry->snir : nullptr;
}

void CommunicationCacheBase::setCachedSNIR(const IRadio *receiver, const ITransmission *transmission, const ISnir *snir)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->snir = snir;
}

void CommunicationCacheBase::removeCachedSNIR(const IRadio *receiver, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->snir = nullptr;
}

const IReceptionDecision *CommunicationCacheBase::getCachedReceptionDecision(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part)
{
    ASSERT(part != IRadioSignal::SIGNAL_PART_NONE);
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    return cacheEntry ? cacheEntry->receptionDecisions[part] : nullptr;
}

void CommunicationCacheBase::setCachedReceptionDecision(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part, const IReceptionDecision *receptionDecision)
{
    ASSERT(part != IRadioSignal::SIGNAL_PART_NONE);
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else {
        delete cacheEntry->receptionDecisions[part];
        cacheEntry->receptionDecisions[part] = receptionDecision;
    }
}

void CommunicationCacheBase::removeCachedReceptionDecision(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part)
{
    ASSERT(part != IRadioSignal::SIGNAL_PART_NONE);
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry != nullptr) {
        delete cacheEntry->receptionDecisions[part];
        cacheEntry->receptionDecisions[part] = nullptr;
    }
}

const IReceptionResult *CommunicationCacheBase::getCachedReceptionResult(const IRadio *receiver, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    return cacheEntry ? cacheEntry->receptionResult : nullptr;
}

void CommunicationCacheBase::setCachedReceptionResult(const IRadio *receiver, const ITransmission *transmission, const IReceptionResult *receptionResult)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->receptionResult = receptionResult;
}

void CommunicationCacheBase::removeCachedReceptionResult(const IRadio *receiver, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->receptionResult = nullptr;
}

const IWirelessSignal *CommunicationCacheBase::getCachedSignal(const IRadio *receiver, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    return cacheEntry ? cacheEntry->signal : nullptr;
}

void CommunicationCacheBase::setCachedSignal(const IRadio *receiver, const ITransmission *transmission, const IWirelessSignal *signal)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry == nullptr)
        throw cRuntimeError("Cache entry not found");
    else
        cacheEntry->signal = signal;
}

void CommunicationCacheBase::removeCachedSignal(const IRadio *receiver, const ITransmission *transmission)
{
    ReceptionCacheEntry *cacheEntry = getReceptionCacheEntry(receiver, transmission);
    if (cacheEntry != nullptr)
        cacheEntry->signal = nullptr;
}

} // namespace physicallayer

} // namespace inet

