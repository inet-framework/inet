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
#include "inet/physicallayer/communicationcache/MapCommunicationCache.h"

namespace inet {

namespace physicallayer {

Define_Module(MapCommunicationCache);

MapCommunicationCache::MapCommunicationCache()
{
}

MapCommunicationCache::~MapCommunicationCache()
{
    for (auto &transmissionIt : transmissionCache)
        delete static_cast<std::map<const IRadio *, ReceptionCacheEntry> *>(transmissionIt.second.receptionCacheEntries);
}

MapCommunicationCache::RadioCacheEntry *MapCommunicationCache::getRadioCacheEntry(const IRadio *radio)
{
    return &radioCache[radio];
}

MapCommunicationCache::TransmissionCacheEntry *MapCommunicationCache::getTransmissionCacheEntry(const ITransmission *transmission)
{
    TransmissionCacheEntry *transmissionCacheEntry = &transmissionCache[transmission];
    if (transmissionCacheEntry->receptionCacheEntries == nullptr)
        transmissionCacheEntry->receptionCacheEntries = new std::map<const IRadio *, ReceptionCacheEntry>();
    return transmissionCacheEntry;
}

MapCommunicationCache::ReceptionCacheEntry *MapCommunicationCache::getReceptionCacheEntry(const IRadio *radio, const ITransmission *transmission)
{
    TransmissionCacheEntry *transmissionCacheEntry = getTransmissionCacheEntry(transmission);
    if (transmissionCacheEntry == nullptr)
        return nullptr;
    else
        return &(*static_cast<std::map<const IRadio *, ReceptionCacheEntry> *>(transmissionCacheEntry->receptionCacheEntries))[radio];
}

void MapCommunicationCache::addRadio(const IRadio *radio)
{
}

void MapCommunicationCache::removeRadio(const IRadio *radio)
{
    auto radioIt = radioCache.find(radio);
    if (radioIt != radioCache.end())
        radioCache.erase(radioIt);
    for (auto &transmissionIt : transmissionCache) {
        std::map<const IRadio *, ReceptionCacheEntry> *receptionCacheEntries = static_cast<std::map<const IRadio *, ReceptionCacheEntry> *>(transmissionIt.second.receptionCacheEntries);
        if (receptionCacheEntries != nullptr) {
            auto receptionIt = receptionCacheEntries->find(radio);
            if (receptionIt != receptionCacheEntries->end())
                receptionCacheEntries->erase(receptionIt);
        }
    }
}

void MapCommunicationCache::addTransmission(const ITransmission *transmission)
{
}

void MapCommunicationCache::removeTransmission(const ITransmission *transmission)
{
    auto transmissionIt = transmissionCache.find(transmission);
    if (transmissionIt != transmissionCache.end()) {
        const TransmissionCacheEntry &transmissionCacheEntry = (*transmissionIt).second;
        std::map<const IRadio *, ReceptionCacheEntry> *receptionCacheEntries = static_cast<std::map<const IRadio *, ReceptionCacheEntry> *>(transmissionCacheEntry.receptionCacheEntries);
        if (receptionCacheEntries != nullptr) {
            for (auto &receptionIt : *receptionCacheEntries) {
                const IRadio *radio = receptionIt.first;
                const RadioCacheEntry &radioCacheEntry = radioCache[radio];
                const ReceptionCacheEntry &recpeionCacheEntry = receptionIt.second;
                const Interval *interval = recpeionCacheEntry.interval;
                if (interval != nullptr)
                    radioCacheEntry.receptionIntervals->deleteNode(interval);
            }
        }
        delete static_cast<std::map<const IRadio *, ReceptionCacheEntry> *>(transmissionCacheEntry.receptionCacheEntries);
        transmissionCache.erase(transmissionIt);
    }
}

void MapCommunicationCache::removeNonInterferingTransmissions()
{
}

} // namespace physicallayer

} // namespace inet

