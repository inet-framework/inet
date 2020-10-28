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

#include "inet/physicallayer/communicationcache/ReferenceCommunicationCache.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

namespace inet {
namespace physicallayer {

Define_Module(ReferenceCommunicationCache);

ReferenceCommunicationCache::~ReferenceCommunicationCache()
{
    for (auto& transmissionCacheEntry : transmissionCache) {
#if OMNETPP_BUILDNUM >= 1505   //OMNETPP_VERSION >= 0x0600    // 6.0 pre9
        delete transmissionCacheEntry.signal;
        delete transmissionCacheEntry.transmission;
#endif
        delete transmissionCacheEntry.receptionCacheEntries;
#if OMNETPP_BUILDNUM < 1505   //OMNETPP_VERSION < 0x0600    // 6.0 pre9
        transmissionCacheEntry.receptionCacheEntries = nullptr;
#endif
    }
}

ReferenceCommunicationCache::RadioCacheEntry *ReferenceCommunicationCache::getRadioCacheEntry(const IRadio *radio)
{
    return &radioCache[radio->getId()];
}

ReferenceCommunicationCache::ReferenceTransmissionCacheEntry *ReferenceCommunicationCache::getTransmissionCacheEntry(const ITransmission *transmission)
{
    return &transmissionCache[transmission->getId()];
}

ReferenceCommunicationCache::ReceptionCacheEntry *ReferenceCommunicationCache::getReceptionCacheEntry(const IRadio *radio, const ITransmission *transmission)
{
    ReferenceTransmissionCacheEntry *transmissionCacheEntry = getTransmissionCacheEntry(transmission);
    auto *receptionCacheEntries = transmissionCacheEntry->receptionCacheEntries;
    int radioId = radio->getId();
    if ((int)receptionCacheEntries->size() <= radioId)
        receptionCacheEntries->resize(radioId + 1);
    return &(*receptionCacheEntries)[radioId];
}

void ReferenceCommunicationCache::addRadio(const IRadio *radio)
{
    int radioId = radio->getId();
    if ((int)radioCache.size() <= radioId)
        radioCache.resize(radioId + 1);
    auto radioCacheEntry = getRadioCacheEntry(radio);
    radioCacheEntry->radio = radio;
}

void ReferenceCommunicationCache::removeRadio(const IRadio *radio)
{
    // NOTE: no erase from the radio cache to test that it doesn't affect simulation results
}

const IRadio *ReferenceCommunicationCache::getRadio(int id) const
{
    for (const auto& radioCacheEntry : radioCache)
        if (radioCacheEntry.radio->getId() == id)
            return radioCacheEntry.radio;
    return nullptr;
}

void ReferenceCommunicationCache::mapRadios(std::function<void (const IRadio *)> f) const
{
    for (const auto& radioCacheEntry : radioCache)
        f(radioCacheEntry.radio);
}

void ReferenceCommunicationCache::addTransmission(const ITransmission *transmission)
{
    int transmissionId = transmission->getId();
    if ((int)transmissionCache.size() <= transmissionId)
        transmissionCache.resize(transmissionId + 1);
    auto transmissionCacheEntry = getTransmissionCacheEntry(transmission);
    transmissionCacheEntry->transmission = transmission;
    transmissionCacheEntry->receptionCacheEntries = new std::vector<ReceptionCacheEntry>(radioCache.size() + 1);
}

void ReferenceCommunicationCache::removeTransmission(const ITransmission *transmission)
{
    // NOTE: no erase from the transmission cache to test that it doesn't affect simulation results
}

const ITransmission *ReferenceCommunicationCache::getTransmission(int id) const
{
    for (const auto& transmissionCacheEntry : transmissionCache)
        if (transmissionCacheEntry.transmission->getId() == id)
            return transmissionCacheEntry.transmission;
    return nullptr;
}

void ReferenceCommunicationCache::mapTransmissions(std::function<void (const ITransmission *)> f) const
{
    for (const auto& transmissionCacheEntry : transmissionCache)
        f(transmissionCacheEntry.transmission);
}

void ReferenceCommunicationCache::removeNonInterferingTransmissions(std::function<void (const ITransmission *transmission)> f)
{
    // NOTE: no erase from the transmission cache to test that it doesn't affect simulation results
}

std::vector<const ITransmission *> *ReferenceCommunicationCache::computeInterferingTransmissions(const IRadio *radio, const simtime_t startTime, const simtime_t endTime)
{
    // NOTE: ignore receptionIntervals to test that it doesn't affect simulation results
    std::vector<const ITransmission *> *interferingTransmissions = new std::vector<const ITransmission *>();
    for (const auto& transmissionCacheEntry : transmissionCache) {
        auto receptionCacheEntries = transmissionCacheEntry.receptionCacheEntries;
        auto& receptionCacheEntry = receptionCacheEntries->at(radio->getId());
        const IArrival *arrival = receptionCacheEntry.arrival;
        if (arrival != nullptr && !(arrival->getEndTime() < startTime || endTime < arrival->getStartTime()))
            interferingTransmissions->push_back(transmissionCacheEntry.transmission);
    }
    return interferingTransmissions;
}

} // namespace physicallayer
} // namespace inet

