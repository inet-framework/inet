//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/communicationcache/MapCommunicationCache.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {
namespace physicallayer {

Define_Module(MapCommunicationCache);

MapCommunicationCache::~MapCommunicationCache()
{
    for (auto& it : transmissionCache)
        delete it.second.receptionCacheEntries;
}

MapCommunicationCache::RadioCacheEntry *MapCommunicationCache::getRadioCacheEntry(const IRadio *radio)
{
    return &radioCache[radio->getId()];
}

MapCommunicationCache::MapTransmissionCacheEntry *MapCommunicationCache::getTransmissionCacheEntry(const ITransmission *transmission)
{
    return &transmissionCache[transmission->getId()];
}

MapCommunicationCache::ReceptionCacheEntry *MapCommunicationCache::getReceptionCacheEntry(const IRadio *radio, const ITransmission *transmission)
{
    MapTransmissionCacheEntry *transmissionCacheEntry = getTransmissionCacheEntry(transmission);
    auto receptionCacheEntries = transmissionCacheEntry->receptionCacheEntries;
    return &(*receptionCacheEntries)[radio->getId()];
}

void MapCommunicationCache::addRadio(const IRadio *radio)
{
    auto radioCacheEntry = getRadioCacheEntry(radio);
    radioCacheEntry->radio = radio;
}

void MapCommunicationCache::removeRadio(const IRadio *radio)
{
    auto radioId = radio->getId();
    auto it = radioCache.find(radioId);
    if (it != radioCache.end())
        radioCache.erase(it);
    for (auto& it : transmissionCache) {
        std::map<int, ReceptionCacheEntry> *receptionCacheEntries = it.second.receptionCacheEntries;
        if (receptionCacheEntries != nullptr) {
            auto jt = receptionCacheEntries->find(radioId);
            if (jt != receptionCacheEntries->end())
                receptionCacheEntries->erase(jt);
        }
    }
}

const IRadio *MapCommunicationCache::getRadio(int id) const
{
    auto it = radioCache.find(id);
    return (it == radioCache.end()) ? nullptr : it->second.radio;
}

void MapCommunicationCache::mapRadios(std::function<void(const IRadio *)> f) const
{
    for (auto& it : radioCache)
        f(it.second.radio);
}

int MapCommunicationCache::getNumRadios() const
{
    return radioCache.size();
}

void MapCommunicationCache::addTransmission(const ITransmission *transmission)
{
    auto transmissionCacheEntry = getTransmissionCacheEntry(transmission);
    transmissionCacheEntry->transmission = transmission;
    transmissionCacheEntry->receptionCacheEntries = new std::map<int, ReceptionCacheEntry>();
}

void MapCommunicationCache::removeTransmission(const ITransmission *transmission)
{
    auto it = transmissionCache.find(transmission->getId());
    if (it != transmissionCache.end()) {
        MapTransmissionCacheEntry& transmissionCacheEntry = it->second;
        std::map<int, ReceptionCacheEntry> *receptionCacheEntries = transmissionCacheEntry.receptionCacheEntries;
        if (receptionCacheEntries != nullptr) {
            for (auto& jt : *receptionCacheEntries) {
                const RadioCacheEntry& radioCacheEntry = radioCache[jt.first];
                const ReceptionCacheEntry& receptionCacheEntry = jt.second;
                const IntervalTree::Interval *interval = receptionCacheEntry.interval;
                if (interval != nullptr)
                    radioCacheEntry.receptionIntervals->deleteNode(interval);
            }
        }
        delete transmissionCacheEntry.receptionCacheEntries;
        transmissionCacheEntry.receptionCacheEntries = nullptr;
        transmissionCache.erase(it);
    }
    else
        throw cRuntimeError("Cannot find transmission");
}

const ITransmission *MapCommunicationCache::getTransmission(int id) const
{
    auto it = transmissionCache.find(id);
    return (it == transmissionCache.end()) ? nullptr : it->second.transmission;
}

void MapCommunicationCache::mapTransmissions(std::function<void(const ITransmission *)> f) const
{
    for (auto& it : transmissionCache)
        f(it.second.transmission);
}

int MapCommunicationCache::getNumTransmissions() const
{
    return transmissionCache.size();
}

void MapCommunicationCache::removeNonInterferingTransmissions(std::function<void(const ITransmission *transmission)> f)
{
    int transmissionCount = 0;
    const simtime_t now = simTime();
    for (auto it = transmissionCache.begin(); it != transmissionCache.end();) {
        MapTransmissionCacheEntry& transmissionCacheEntry = it->second;
        if (transmissionCacheEntry.interferenceEndTime <= now) {
            if (transmissionCacheEntry.receptionCacheEntries != nullptr) {
                std::map<int, ReceptionCacheEntry> *receptionCacheEntries = transmissionCacheEntry.receptionCacheEntries;
                for (auto radioIt = radioCache.cbegin(); radioIt != radioCache.cend(); radioIt++) {
                    const RadioCacheEntry& radioCacheEntry = radioIt->second;
                    auto receptionIt = receptionCacheEntries->find(radioIt->first);
                    if (receptionIt != receptionCacheEntries->cend()) {
                        const ReceptionCacheEntry& receptionCacheEntry = receptionIt->second;
                        const IntervalTree::Interval *interval = receptionCacheEntry.interval;
                        if (interval != nullptr)
                            radioCacheEntry.receptionIntervals->deleteNode(interval);
                    }
                }
                delete receptionCacheEntries;
                transmissionCacheEntry.receptionCacheEntries = nullptr;
            }
            f(transmissionCacheEntry.transmission);
            transmissionCache.erase(it++);
            transmissionCount++;
        }
        else
            it++;
    }
    EV_DEBUG << "Removed " << transmissionCount << " non interfering transmissions\n";
}

} // namespace physicallayer
} // namespace inet

