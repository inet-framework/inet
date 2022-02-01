//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/communicationcache/VectorCommunicationCache.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {
namespace physicallayer {

Define_Module(VectorCommunicationCache);

VectorCommunicationCache::~VectorCommunicationCache()
{
    for (auto& transmissionCacheEntry : transmissionCache)
        delete transmissionCacheEntry.receptionCacheEntries;
}

VectorCommunicationCache::RadioCacheEntry *VectorCommunicationCache::getRadioCacheEntry(const IRadio *radio)
{
    ASSERT(baseRadioId != -1);
    int radioIndex = radio->getId() - baseRadioId;
    ASSERT(0 <= radioIndex && radioIndex < (int)radioCache.size());
    return &radioCache[radioIndex];
}

VectorCommunicationCache::VectorTransmissionCacheEntry *VectorCommunicationCache::getTransmissionCacheEntry(const ITransmission *transmission)
{
    ASSERT(baseTransmissionId != -1);
    int transmissionIndex = transmission->getId() - baseTransmissionId;
    ASSERT(0 <= transmissionIndex && transmissionIndex < (int)transmissionCache.size());
    return &transmissionCache[transmissionIndex];
}

VectorCommunicationCache::ReceptionCacheEntry *VectorCommunicationCache::getReceptionCacheEntry(const IRadio *radio, const ITransmission *transmission)
{
    VectorTransmissionCacheEntry *transmissionCacheEntry = getTransmissionCacheEntry(transmission);
    if (transmissionCacheEntry == nullptr)
        return nullptr;
    else {
        ASSERT(baseRadioId != -1);
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
    int radioId = radio->getId();
    if (baseRadioId == -1)
        baseRadioId = radioId;
    int radioIndex = radio->getId() - baseRadioId;
    if (radioIndex < 0) {
        radioCache.insert(radioCache.begin(), -radioIndex, RadioCacheEntry());
        for (auto& transmissionCacheEntry : transmissionCache) {
            std::vector<ReceptionCacheEntry> *receptionCacheEntries = transmissionCacheEntry.receptionCacheEntries;
            if (receptionCacheEntries != nullptr)
                receptionCacheEntries->insert(receptionCacheEntries->begin(), -radioIndex, ReceptionCacheEntry());
        }
        baseRadioId = radioId;
    }
    else if (radioIndex >= (int)radioCache.size())
        radioCache.resize(radioIndex + 1);
    auto radioCacheEntry = getRadioCacheEntry(radio);
    radioCacheEntry->radio = radio;
}

void VectorCommunicationCache::removeRadio(const IRadio *radio)
{
    auto radioCacheEntry = getRadioCacheEntry(radio);
    radioCacheEntry->radio = nullptr;
    int radioCount = 0;
    while (radioCache[radioCount].radio == nullptr && radioCount < (int)radioCache.size())
        radioCount++;
    if (radioCount != 0) {
        ASSERT(baseRadioId != -1);
        baseRadioId += radioCount;
        radioCache.erase(radioCache.begin(), radioCache.begin() + radioCount);
        for (auto& transmissionCacheEntry : transmissionCache) {
            std::vector<ReceptionCacheEntry> *receptionCacheEntries = transmissionCacheEntry.receptionCacheEntries;
            if (receptionCacheEntries != nullptr)
                receptionCacheEntries->erase(receptionCacheEntries->begin(), receptionCacheEntries->begin() + radioCount);
        }
    }
}

const IRadio *VectorCommunicationCache::getRadio(int id) const
{
    ASSERT(baseRadioId != -1);
    int radioIndex = id - baseRadioId;
    if (0 <= radioIndex && radioIndex < (int)radioCache.size())
        return radioCache[radioIndex].radio;
    else
        return nullptr;
}

void VectorCommunicationCache::mapRadios(std::function<void(const IRadio *)> f) const
{
    for (auto& radioCacheEntry : radioCache)
        if (radioCacheEntry.radio != nullptr)
            f(radioCacheEntry.radio);
}

void VectorCommunicationCache::addTransmission(const ITransmission *transmission)
{
    int transmissionId = transmission->getId();
    if (baseTransmissionId == -1)
        baseTransmissionId = transmissionId;
    int transmissionIndex = transmission->getId() - baseTransmissionId;
    if (transmissionIndex < 0) {
        transmissionCache.insert(transmissionCache.begin(), -transmissionIndex, VectorTransmissionCacheEntry());
        baseTransmissionId = transmissionId;
    }
    else if (transmissionIndex >= (int)transmissionCache.size())
        transmissionCache.resize(transmissionIndex + 1);
    auto transmissionCacheEntry = getTransmissionCacheEntry(transmission);
    transmissionCacheEntry->transmission = transmission;
    transmissionCacheEntry->receptionCacheEntries = new std::vector<ReceptionCacheEntry>(radioCache.size());
}

void VectorCommunicationCache::removeTransmission(const ITransmission *transmission)
{
    ASSERT(baseTransmissionId != -1);
    int transmissionIndex = transmission->getId() - baseTransmissionId;
    if (0 <= transmissionIndex && transmissionIndex < (int)transmissionCache.size())
        transmissionCache[transmissionIndex] = VectorTransmissionCacheEntry();
    else
        throw cRuntimeError("Cannot find transmission");
}

const ITransmission *VectorCommunicationCache::getTransmission(int id) const
{
    ASSERT(baseTransmissionId != -1);
    int transmissionIndex = id - baseTransmissionId;
    if (0 <= transmissionIndex && transmissionIndex < (int)transmissionCache.size())
        return transmissionCache[transmissionIndex].transmission;
    else
        return nullptr;
}

void VectorCommunicationCache::mapTransmissions(std::function<void(const ITransmission *)> f) const
{
    for (auto& transmissionCacheEntry : transmissionCache)
        if (transmissionCacheEntry.transmission != nullptr)
            f(transmissionCacheEntry.transmission);
}

void VectorCommunicationCache::removeNonInterferingTransmissions(std::function<void(const ITransmission *transmission)> f)
{
    const simtime_t now = simTime();
    size_t transmissionIndex = 0;
    for (auto it = transmissionCache.begin(); it != transmissionCache.end(); ++it) {
        VectorTransmissionCacheEntry& transmissionCacheEntry = *it;
        if (transmissionCacheEntry.interferenceEndTime <= now) {
            transmissionIndex++;
            if (transmissionCacheEntry.receptionCacheEntries != nullptr) {
                std::vector<ReceptionCacheEntry> *receptionCacheEntries = transmissionCacheEntry.receptionCacheEntries;
                auto radioIt = radioCache.cbegin();
                auto receptionIt = receptionCacheEntries->cbegin();
                while (radioIt != radioCache.cend() && receptionIt != receptionCacheEntries->cend()) {
                    const RadioCacheEntry& radioCacheEntry = *radioIt;
                    const ReceptionCacheEntry& receptionCacheEntry = *receptionIt;
                    const IntervalTree::Interval *interval = receptionCacheEntry.interval;
                    if (interval != nullptr)
                        radioCacheEntry.receptionIntervals->deleteNode(interval);
                    radioIt++;
                    receptionIt++;
                }
                delete receptionCacheEntries;
                transmissionCacheEntry.receptionCacheEntries = nullptr;
            }
            if (transmissionCacheEntry.transmission != nullptr)
                f(transmissionCacheEntry.transmission);
        }
        else
            break;
    }
    ASSERT(baseTransmissionId != -1);
    baseTransmissionId += transmissionIndex;
    transmissionCache.erase(transmissionCache.begin(), transmissionCache.begin() + transmissionIndex);
    EV_DEBUG << "Removed " << transmissionIndex << " non interfering transmissions\n";
}

} // namespace physicallayer
} // namespace inet

