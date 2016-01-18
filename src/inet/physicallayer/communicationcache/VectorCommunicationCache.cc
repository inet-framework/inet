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
#include "inet/physicallayer/communicationcache/VectorCommunicationCache.h"

namespace inet {

namespace physicallayer {

Define_Module(VectorCommunicationCache);

VectorCommunicationCache::VectorCommunicationCache() :
    baseRadioId(0),
    baseTransmissionId(0)
{
}

VectorCommunicationCache::~VectorCommunicationCache()
{
    for (auto &transmissionCacheEntry : transmissionCache)
        delete static_cast<std::vector<ReceptionCacheEntry> *>(transmissionCacheEntry.receptionCacheEntries);
}

VectorCommunicationCache::RadioCacheEntry *VectorCommunicationCache::getRadioCacheEntry(const IRadio *radio)
{
    int radioIndex = radio->getId() - baseRadioId;
    if (radioIndex < 0)
        return nullptr;
    else {
        if (radioIndex >= (int)radioCache.size())
            radioCache.resize(radioIndex + 1);
        return &radioCache[radioIndex];
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
        if (transmissionCacheEntry.receptionCacheEntries == nullptr)
            transmissionCacheEntry.receptionCacheEntries = new std::vector<ReceptionCacheEntry>(radioCache.size());
        return &transmissionCacheEntry;
    }
}

VectorCommunicationCache::ReceptionCacheEntry *VectorCommunicationCache::getReceptionCacheEntry(const IRadio *radio, const ITransmission *transmission)
{
    TransmissionCacheEntry *transmissionCacheEntry = getTransmissionCacheEntry(transmission);
    if (transmissionCacheEntry == nullptr)
        return nullptr;
    else {
        std::vector<ReceptionCacheEntry> *receptionCacheEntries = static_cast<std::vector<ReceptionCacheEntry> *>(transmissionCacheEntry->receptionCacheEntries);
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
    getRadioCacheEntry(radio)->stale = true;
    int radioCount = 0;
    while (radioCount < (int)radioCache.size() && radioCache[radioCount].stale)
        radioCount++;
    if (radioCount != 0) {
        baseRadioId += radioCount;
        radioCache.erase(radioCache.begin(), radioCache.begin() + radioCount);
        for (auto &transmissionCacheEntry : transmissionCache) {
            std::vector<ReceptionCacheEntry> *receptionCacheEntries = static_cast<std::vector<ReceptionCacheEntry> *>(transmissionCacheEntry.receptionCacheEntries);
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

void VectorCommunicationCache::removeNonInterferingTransmissions()
{
    const simtime_t now = simTime();
    size_t transmissionIndex = 0;
    while (transmissionIndex < transmissionCache.size() && transmissionCache[transmissionIndex].interferenceEndTime <= now)
        transmissionIndex++;
    for (auto it = transmissionCache.cbegin(); it != transmissionCache.cbegin() + transmissionIndex; it++) {
        const TransmissionCacheEntry &transmissionCacheEntry = *it;
        if (transmissionCacheEntry.receptionCacheEntries != nullptr) {
            std::vector<ReceptionCacheEntry> *receptionCacheEntries = static_cast<std::vector<ReceptionCacheEntry> *>(transmissionCacheEntry.receptionCacheEntries);
            auto radioIt = radioCache.cbegin();
            auto receptionIt = receptionCacheEntries->cbegin();
            while (radioIt != radioCache.cend() && receptionIt != receptionCacheEntries->cend()) {
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
        delete static_cast<std::vector<ReceptionCacheEntry> *>(transmissionCacheEntry.receptionCacheEntries);
    }
    transmissionCache.erase(transmissionCache.begin(), transmissionCache.begin() + transmissionIndex);
}

} // namespace physicallayer

} // namespace inet

