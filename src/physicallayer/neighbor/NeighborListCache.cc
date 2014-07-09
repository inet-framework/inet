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

#include "NeighborListCache.h"
#include "ModuleAccess.h"

namespace inet {

namespace physicallayer {

Define_Module(NeighborListCache);
void NeighborListCache::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        // TODO: NED parameter?
        radioMedium = getModuleFromPar<RadioMedium>(par("radioMediumModule"), this);
        updatePeriod = par("refillPeriod");
        range = par("range");
        updateNeighborListsTimer = new cMessage("updateNeighborListsTimer");
    }
    else if (stage == INITSTAGE_LINK_LAYER_2) {    // TODO: is it the correct stage to do this?
        maxSpeed = radioMedium->getMaxSpeed().get();
        updateNeighborLists();
    }
}

void NeighborListCache::sendToNeighbors(IRadio *transmitter, const IRadioFrame *frame, double range)
{
    if (this->range < range)
        throw cRuntimeError("The transmitter's (id: %d) range is bigger then the global NeighborListCache range", transmitter->getId());

    RadioEntryCache::iterator it = radioToEntry.find(transmitter);
    if (it == radioToEntry.end())
        throw cRuntimeError("Transmitter is not found");

    RadioEntry *radioEntry = it->second;
    Radios& neighborVector = radioEntry->neighborVector;

    for (unsigned int i = 0; i < neighborVector.size(); i++)
        radioMedium->sendToRadio(transmitter, neighborVector[i], frame);
}

void NeighborListCache::handleMessage(cMessage *msg)
{
    if (!msg->isSelfMessage())
        throw cRuntimeError("This module only handles self messages");

    updateNeighborLists();

    scheduleAt(simTime() + updatePeriod, msg);
}

void NeighborListCache::updateNeighborList(RadioEntry *radioEntry)
{
    IMobility *radioMobility = radioEntry->radio->getAntenna()->getMobility();
    Coord radioPosition = radioMobility->getCurrentPosition();
    double radius = maxSpeed * updatePeriod + range;
    radioEntry->neighborVector.clear();

    for (unsigned int i = 0; i < radios.size(); i++) {
        const IRadio *otherRadio = radios[i]->radio;
        Coord otherEntryPosition = otherRadio->getAntenna()->getMobility()->getCurrentPosition();

        if (otherRadio->getId() != radioEntry->radio->getId() &&
            otherEntryPosition.sqrdist(radioPosition) <= radius * radius)
            radioEntry->neighborVector.push_back(otherRadio);
    }
}

void NeighborListCache::addRadio(const IRadio *radio)
{
    RadioEntry *newEntry = new RadioEntry(radio);
    radios.push_back(newEntry);
    radioToEntry[radio] = newEntry;
    updateNeighborLists();
    maxSpeed = radioMedium->getMaxSpeed().get();
    if (maxSpeed != 0 && !updateNeighborListsTimer->isScheduled() && initialized())
        scheduleAt(simTime() + updatePeriod, updateNeighborListsTimer);
}

void NeighborListCache::removeRadio(const IRadio *radio)
{
    RadioEntries::iterator it = find(radios.begin(), radios.end(), radioToEntry[radio]);
    if (it != radios.end()) {
        removeRadioFromNeighborLists(radio);
        radios.erase(it);
        maxSpeed = radioMedium->getMaxSpeed().get();
        if (maxSpeed == 0)
            cancelEvent(updateNeighborListsTimer);
    }
    else {
        throw cRuntimeError("You can't remove radio: %d because it is not in our radio vector", radio->getId());
    }
}

void NeighborListCache::updateNeighborLists()
{
    EV_DETAIL << "Updating the neighbor lists" << endl;
    for (unsigned int i = 0; i < radios.size(); i++)
        updateNeighborList(radios[i]);
}

void NeighborListCache::removeRadioFromNeighborLists(const IRadio *radio)
{
    for (unsigned int i = 0; i < radios.size(); i++) {
        Radios neighborVector = radios[i]->neighborVector;
        Radios::iterator it = find(neighborVector.begin(), neighborVector.end(), radio);
        if (it != neighborVector.end())
            neighborVector.erase(it);
    }
}

NeighborListCache::~NeighborListCache()
{
    for (unsigned int i = 0; i < radios.size(); i++)
        delete radios[i];

    cancelAndDelete(updateNeighborListsTimer);
}

} // namespace physicallayer

} // namespace inet

