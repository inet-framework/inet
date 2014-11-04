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

#include "inet/physicallayer/neighborcache/QuadTreeNeighborCache.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

namespace physicallayer {

Define_Module(QuadTreeNeighborCache);

QuadTreeNeighborCache::QuadTreeNeighborCache() :
    quadTree(NULL),
    radioMedium(NULL),
    rebuildQuadTreeTimer(NULL),
    constraintAreaMax(Coord::NIL),
    constraintAreaMin(Coord::NIL),
    maxNumOfPointsPerQuadrant(0),
    refillPeriod(NaN),
    maxSpeed(NaN)
{
}

void QuadTreeNeighborCache::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        radioMedium = getModuleFromPar<RadioMedium>(par("radioMediumModule"), this);
        rebuildQuadTreeTimer = new cMessage("rebuildQuadTreeTimer");
        refillPeriod = par("refillPeriod");
        maxNumOfPointsPerQuadrant = par("maxNumOfPointsPerQuadrant");
    }
    else if (stage == INITSTAGE_LINK_LAYER_2) {
        constraintAreaMax = radioMedium->getConstraintAreaMax();
        constraintAreaMin = radioMedium->getConstraintAreaMin();
        quadTree = new QuadTree(constraintAreaMin, constraintAreaMax, maxNumOfPointsPerQuadrant, NULL);
        maxSpeed = radioMedium->getMaxSpeed().get();
        rebuildQuadTree();
        scheduleAt(simTime() + refillPeriod, rebuildQuadTreeTimer);
    }
}

void QuadTreeNeighborCache::handleMessage(cMessage *msg)
{
    if (!msg->isSelfMessage())
        throw cRuntimeError("This module only handles self messages");
    rebuildQuadTree();
    scheduleAt(simTime() + refillPeriod, msg);
}

void QuadTreeNeighborCache::printToStream(std::ostream& stream) const
{
    stream << "QuadTreeNeighborCache, "
           << "maxNumOfPointsPerQuadrant = " << maxNumOfPointsPerQuadrant << ", "
           << "refillPeriod = " << refillPeriod << ", "
           << "maxSpeed = " << maxSpeed;
}

void QuadTreeNeighborCache::addRadio(const IRadio *radio)
{
    radios.push_back(radio);
    Coord radioPos = radio->getAntenna()->getMobility()->getCurrentPosition();
    maxSpeed = radioMedium->getMaxSpeed().get();
    if (maxSpeed != 0 && !rebuildQuadTreeTimer->isScheduled() && initialized())
        scheduleAt(simTime() + refillPeriod, rebuildQuadTreeTimer);
    Coord newConstraintAreaMin = radioMedium->getConstraintAreaMin();
    Coord newConstraintAreaMax = radioMedium->getConstraintAreaMax();
    // If the constraintArea changed we must rebuild the QuadTree
    if (newConstraintAreaMin != constraintAreaMin || newConstraintAreaMax != constraintAreaMax)
    {
        constraintAreaMin = newConstraintAreaMin;
        constraintAreaMax = newConstraintAreaMax;
        if (initialized())
            rebuildQuadTree();
    }
    else if (initialized())
        quadTree->insert(check_and_cast<const cObject *>(radio), radioPos);
}

void QuadTreeNeighborCache::removeRadio(const IRadio *radio)
{
    Radios::iterator it = find(radios.begin(), radios.end(), radio);
    if (it != radios.end()) {
        radios.erase(it);
        Coord newConstraintAreaMin = radioMedium->getConstraintAreaMin();
        Coord newConstraintAreaMax = radioMedium->getConstraintAreaMax();
        if (newConstraintAreaMin != constraintAreaMin || newConstraintAreaMax != constraintAreaMax)
        {
            constraintAreaMin = newConstraintAreaMin;
            constraintAreaMax = newConstraintAreaMax;
            if (initialized())
                rebuildQuadTree();
        }
        else if (initialized())
            quadTree->remove(check_and_cast<const cObject *>(radio));
        maxSpeed = radioMedium->getMaxSpeed().get();
        if (maxSpeed == 0 && initialized())
            cancelEvent(rebuildQuadTreeTimer);
    }
    else
        throw cRuntimeError("You can't remove radio: %d because it is not in our radio container", radio->getId());
}

void QuadTreeNeighborCache::sendToNeighbors(IRadio *transmitter, const IRadioFrame *frame, double range) const
{
    double radius = range + refillPeriod * maxSpeed;
    Coord transmitterPos = transmitter->getAntenna()->getMobility()->getCurrentPosition();
    QuadTreeNeighborCacheVisitor visitor(radioMedium, transmitter, frame);
    quadTree->rangeQuery(transmitterPos, radius, &visitor);
}

void QuadTreeNeighborCache::fillQuadTreeWithRadios()
{
    for (unsigned int i = 0; i < radios.size(); i++) {
        Coord radioPos = radios[i]->getAntenna()->getMobility()->getCurrentPosition();
        if (!quadTree->insert(check_and_cast<const cObject *>(radios[i]), radioPos))
            throw cRuntimeError("Unsuccessful QuadTree building");
    }
}

void QuadTreeNeighborCache::rebuildQuadTree()
{
    delete quadTree;
    quadTree = new QuadTree(constraintAreaMin, constraintAreaMax, maxNumOfPointsPerQuadrant, NULL);
    fillQuadTreeWithRadios();
}

QuadTreeNeighborCache::~QuadTreeNeighborCache()
{
    delete quadTree;
    cancelAndDelete(rebuildQuadTreeTimer);
}

void QuadTreeNeighborCache::QuadTreeNeighborCacheVisitor::visit(const cObject *radio) const
{
    const IRadio *neighbor = check_and_cast<const IRadio *>(radio);
    if (neighbor->getId() != transmitter->getId())
        radioMedium->sendToRadio(transmitter, neighbor, frame);
}

} // namespace physicallayer

} // namespace inet

