//
// Copyright (C) OpenSim Ltd.
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

#include "inet/physicallayer/communicationcache/MapCommunicationCache.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

namespace inet {
namespace physicallayer {

Define_Module(MapCommunicationCache);

MapCommunicationCache::~MapCommunicationCache()
{
    for (auto& it : transmissionCache) {
#if OMNETPP_BUILDNUM < 1505   //OMNETPP_VERSION < 0x0600    // 6.0 pre9
        delete it.second.receptionCacheEntries;
        it.second.receptionCacheEntries = nullptr;
#else
        auto& transmissionCacheEntry = it.second;
        delete transmissionCacheEntry.signal;
        delete transmissionCacheEntry.transmission;
        delete transmissionCacheEntry.receptionCacheEntries;
#endif
    }
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
    if (it == radioCache.end())
        return nullptr;
    else
        return it->second.radio;
}

void MapCommunicationCache::mapRadios(std::function<void (const IRadio *)> f) const
{
    for (auto& it : radioCache)
        f(it.second.radio);
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
#if OMNETPP_BUILDNUM >= 1505   //OMNETPP_VERSION >= 0x0600    // 6.0 pre9
        delete transmissionCacheEntry.signal;
#endif
        transmissionCache.erase(it);
    }
#if OMNETPP_BUILDNUM >= 1505   //OMNETPP_VERSION >= 0x0600    // 6.0 pre9
    delete transmission;
#endif
}

const ITransmission *MapCommunicationCache::getTransmission(int id) const
{
    auto it = transmissionCache.find(id);
    if (it == transmissionCache.end())
        return nullptr;
    else
        return it->second.transmission;
}

void MapCommunicationCache::mapTransmissions(std::function<void (const ITransmission *)> f) const
{
    for (auto& it : transmissionCache)
        f(it.second.transmission);
}

void MapCommunicationCache::removeNonInterferingTransmissions(std::function<void (const ITransmission *transmission)> f)
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
#if OMNETPP_BUILDNUM >= 1505   //OMNETPP_VERSION >= 0x0600    // 6.0 pre9
            delete transmissionCacheEntry.signal;
            delete transmissionCacheEntry.transmission;
#endif
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

