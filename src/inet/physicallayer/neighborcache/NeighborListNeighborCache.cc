//
// Copyright (C) 2014 OpenSim Ltd.
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

#include "inet/common/ModuleAccess.h"
#include "inet/physicallayer/neighborcache/NeighborListNeighborCache.h"

namespace inet {
namespace physicallayer {

Define_Module(NeighborListNeighborCache);

NeighborListNeighborCache::NeighborListNeighborCache() :
    radioMedium(nullptr),
    updateNeighborListsTimer(nullptr),
    refillPeriod(NaN),
    range(NaN),
    maxSpeed(NaN)
{
}

void NeighborListNeighborCache::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        radioMedium = getModuleFromPar<RadioMedium>(par("radioMediumModule"), this);
        refillPeriod = par("refillPeriod");
        range = par("range");
        updateNeighborListsTimer = new cMessage("updateNeighborListsTimer");
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER_NEIGHBOR_CACHE) {
        maxSpeed = radioMedium->getMediumLimitCache()->getMaxSpeed().get();
        updateNeighborLists();
        scheduleAfter(refillPeriod, updateNeighborListsTimer);
    }
}

std::ostream& NeighborListNeighborCache::printToStream(std::ostream& stream, int level) const
{
    stream << "NeighborListNeighborCache";
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", refillPeriod = " << refillPeriod
               << ", range = " << range
               << ", maxSpeed = " << maxSpeed;
    return stream;
}

void NeighborListNeighborCache::sendToNeighbors(IRadio *transmitter, const IWirelessSignal *signal, double range) const
{
    if (this->range < range)
        throw cRuntimeError("The transmitter's (id: %d) range is bigger then the cache range", transmitter->getId());

    RadioEntryCache::const_iterator it = radioToEntry.find(transmitter);
    if (it == radioToEntry.end())
        throw cRuntimeError("Transmitter is not found");

    RadioEntry *radioEntry = it->second;
    Radios& neighborVector = radioEntry->neighborVector;

    for (auto & elem : neighborVector)
        radioMedium->sendToRadio(transmitter, elem, signal);
}

void NeighborListNeighborCache::handleMessage(cMessage *msg)
{
    if (!msg->isSelfMessage())
        throw cRuntimeError("This module only handles self messages");

    updateNeighborLists();

    scheduleAfter(refillPeriod, msg);
}

void NeighborListNeighborCache::updateNeighborList(RadioEntry *radioEntry)
{
    IMobility *radioMobility = radioEntry->radio->getAntenna()->getMobility();
    Coord radioPosition = radioMobility->getCurrentPosition();
    double radius = maxSpeed * refillPeriod + range;
    radioEntry->neighborVector.clear();

    for (auto & elem : radios) {
        const IRadio *otherRadio = elem->radio;
        Coord otherEntryPosition = otherRadio->getAntenna()->getMobility()->getCurrentPosition();

        if (otherRadio->getId() != radioEntry->radio->getId() &&
            otherEntryPosition.sqrdist(radioPosition) <= radius * radius)
            radioEntry->neighborVector.push_back(otherRadio);
    }
}

void NeighborListNeighborCache::addRadio(const IRadio *radio)
{
    RadioEntry *newEntry = new RadioEntry(radio);
    radios.push_back(newEntry);
    radioToEntry[radio] = newEntry;
    updateNeighborLists();
    maxSpeed = radioMedium->getMediumLimitCache()->getMaxSpeed().get();
    if (maxSpeed != 0 && !updateNeighborListsTimer->isScheduled() && initialized())
        scheduleAfter(refillPeriod, updateNeighborListsTimer);
}

void NeighborListNeighborCache::removeRadio(const IRadio *radio)
{
    auto it = find(radios.begin(), radios.end(), radioToEntry[radio]);
    if (it != radios.end()) {
        removeRadioFromNeighborLists(radio);
        radios.erase(it);
        maxSpeed = radioMedium->getMediumLimitCache()->getMaxSpeed().get();
        if (maxSpeed == 0 && initialized())
            cancelEvent(updateNeighborListsTimer);
    }
    else {
        throw cRuntimeError("You can't remove radio: %d because it is not in our radio vector", radio->getId());
    }
}

void NeighborListNeighborCache::updateNeighborLists()
{
    EV_DETAIL << "Updating the neighbor lists" << endl;
    for (auto & elem : radios)
        updateNeighborList(elem);
}

void NeighborListNeighborCache::removeRadioFromNeighborLists(const IRadio *radio)
{
    for (auto & elem : radios) {
        Radios neighborVector = elem->neighborVector;
        auto it = find(neighborVector.begin(), neighborVector.end(), radio);
        if (it != neighborVector.end())
            neighborVector.erase(it);
    }
}

NeighborListNeighborCache::~NeighborListNeighborCache()
{
    for (auto & elem : radios)
        delete elem;

    cancelAndDelete(updateNeighborListsTimer);
}

} // namespace physicallayer
} // namespace inet

