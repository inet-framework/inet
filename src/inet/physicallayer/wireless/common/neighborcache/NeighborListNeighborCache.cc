//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/neighborcache/NeighborListNeighborCache.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/stlutils.h"

namespace inet {
namespace physicallayer {

Define_Module(NeighborListNeighborCache);

NeighborListNeighborCache::NeighborListNeighborCache() :
    updateNeighborListsTimer(nullptr),
    refillPeriod(NaN),
    range(NaN),
    maxSpeed(NaN)
{
}

void NeighborListNeighborCache::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        radioMedium.reference(this, "radioMediumModule", true);
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

std::ostream& NeighborListNeighborCache::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "NeighborListNeighborCache";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(refillPeriod)
               << EV_FIELD(range)
               << EV_FIELD(maxSpeed);
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

    for (auto& elem : neighborVector)
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

    for (auto& elem : radios) {
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
    auto it = find(radios, radioToEntry[radio]);
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
    for (auto& elem : radios)
        updateNeighborList(elem);
}

void NeighborListNeighborCache::removeRadioFromNeighborLists(const IRadio *radio)
{
    for (auto& elem : radios) {
        Radios neighborVector = elem->neighborVector;
        auto it = find(neighborVector, radio);
        if (it != neighborVector.end())
            neighborVector.erase(it);
    }
}

NeighborListNeighborCache::~NeighborListNeighborCache()
{
    for (auto& elem : radios)
        delete elem;

    cancelAndDelete(updateNeighborListsTimer);
}

} // namespace physicallayer
} // namespace inet

