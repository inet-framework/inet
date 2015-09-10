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
#include "inet/physicallayer/communicationcache/ReferenceCommunicationCache.h"

namespace inet {

namespace physicallayer {

Define_Module(ReferenceCommunicationCache);

ReferenceCommunicationCache::ReferenceCommunicationCache()
{
}

ReferenceCommunicationCache::~ReferenceCommunicationCache()
{
}

ReferenceCommunicationCache::RadioCacheEntry *ReferenceCommunicationCache::getRadioCacheEntry(const IRadio *radio)
{
    int radioId = radio->getId();
    if ((int)radioCache.size() < radioId)
        radioCache.resize(radioId);
    return &radioCache[radioId];
}

ReferenceCommunicationCache::TransmissionCacheEntry *ReferenceCommunicationCache::getTransmissionCacheEntry(const ITransmission *transmission)
{
    int transmissionId = transmission->getId();
    if ((int)transmissionCache.size() < transmissionId)
        transmissionCache.resize(transmissionId);
    TransmissionCacheEntry *transmissionCacheEntry = &transmissionCache[transmissionId];
    if (transmissionCacheEntry->receptionCacheEntries == nullptr)
        transmissionCacheEntry->receptionCacheEntries = new std::vector<ReceptionCacheEntry>(radioCache.size());
    return transmissionCacheEntry;
}

ReferenceCommunicationCache::ReceptionCacheEntry *ReferenceCommunicationCache::getReceptionCacheEntry(const IRadio *radio, const ITransmission *transmission)
{
    TransmissionCacheEntry *transmissionCacheEntry = getTransmissionCacheEntry(transmission);
    std::vector<ReceptionCacheEntry> *receptionCacheEntries = static_cast<std::vector<ReceptionCacheEntry> *>(transmissionCacheEntry->receptionCacheEntries);
    int radioId = radio->getId();
    if ((int)receptionCacheEntries->size() < radioId)
        receptionCacheEntries->resize(radioId);
    return &(*receptionCacheEntries)[radioId];
}

void ReferenceCommunicationCache::addRadio(const IRadio *radio)
{
    radioCache.push_back(RadioCacheEntry());
}

void ReferenceCommunicationCache::removeRadio(const IRadio *radio)
{
}

void ReferenceCommunicationCache::addTransmission(const ITransmission *transmission)
{
    transmissionCache.push_back(TransmissionCacheEntry());
}

void ReferenceCommunicationCache::removeTransmission(const ITransmission *transmission)
{
}

void ReferenceCommunicationCache::removeNonInterferingTransmissions()
{
}

std::vector<const ITransmission *> *ReferenceCommunicationCache::computeInterferingTransmissions(const IRadio *radio, const simtime_t startTime, const simtime_t endTime)
{
    std::vector<const ITransmission *> *interferingTransmissions = new std::vector<const ITransmission *>();
    for (const auto &transmissionCacheEntry : transmissionCache) {
        for (const auto &receptionCacheEntry : *static_cast<std::vector<ReceptionCacheEntry> *>(transmissionCacheEntry.receptionCacheEntries)) {
            const IArrival *arrival = receptionCacheEntry.arrival;
            if (arrival != nullptr && !(arrival->getEndTime() < startTime || endTime < arrival->getStartTime()))
                interferingTransmissions->push_back(transmissionCacheEntry.frame->getTransmission());
        }
    }
    return interferingTransmissions;
}

} // namespace physicallayer

} // namespace inet

