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
#include "inet/physicallayer/neighborcache/QuadTreeNeighborCache.h"

namespace inet {
namespace physicallayer {

Define_Module(QuadTreeNeighborCache);

QuadTreeNeighborCache::QuadTreeNeighborCache() :
    quadTree(nullptr),
    radioMedium(nullptr),
    rebuildQuadTreeTimer(nullptr),
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
    else if (stage == INITSTAGE_PHYSICAL_LAYER_NEIGHBOR_CACHE) {
        constraintAreaMin = radioMedium->getMediumLimitCache()->getMinConstraintArea();
        constraintAreaMax = radioMedium->getMediumLimitCache()->getMaxConstraintArea();
        quadTree = new QuadTree(constraintAreaMin, constraintAreaMax, maxNumOfPointsPerQuadrant, nullptr);
        maxSpeed = radioMedium->getMediumLimitCache()->getMaxSpeed().get();
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

std::ostream& QuadTreeNeighborCache::printToStream(std::ostream& stream, int level) const
{
    stream << "QuadTreeNeighborCache";
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", maxNumOfPointsPerQuadrant = " << maxNumOfPointsPerQuadrant
               << ", refillPeriod = " << refillPeriod
               << ", maxSpeed = " << maxSpeed;
    return stream;
}

void QuadTreeNeighborCache::addRadio(const IRadio *radio)
{
    radios.push_back(radio);
    Coord radioPos = radio->getAntenna()->getMobility()->getCurrentPosition();
    maxSpeed = radioMedium->getMediumLimitCache()->getMaxSpeed().get();
    if (maxSpeed != 0 && !rebuildQuadTreeTimer->isScheduled() && initialized())
        scheduleAt(simTime() + refillPeriod, rebuildQuadTreeTimer);
    Coord newConstraintAreaMin = radioMedium->getMediumLimitCache()->getMinConstraintArea();
    Coord newConstraintAreaMax = radioMedium->getMediumLimitCache()->getMaxConstraintArea();
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
    auto it = find(radios.begin(), radios.end(), radio);
    if (it != radios.end()) {
        radios.erase(it);
        Coord newConstraintAreaMin = radioMedium->getMediumLimitCache()->getMinConstraintArea();
        Coord newConstraintAreaMax = radioMedium->getMediumLimitCache()->getMaxConstraintArea();
        if (newConstraintAreaMin != constraintAreaMin || newConstraintAreaMax != constraintAreaMax)
        {
            constraintAreaMin = newConstraintAreaMin;
            constraintAreaMax = newConstraintAreaMax;
            if (initialized())
                rebuildQuadTree();
        }
        else if (initialized())
            quadTree->remove(check_and_cast<const cObject *>(radio));
        maxSpeed = radioMedium->getMediumLimitCache()->getMaxSpeed().get();
        if (maxSpeed == 0 && initialized())
            cancelEvent(rebuildQuadTreeTimer);
    }
    else
        throw cRuntimeError("You can't remove radio: %d because it is not in our radio container", radio->getId());
}

void QuadTreeNeighborCache::sendToNeighbors(IRadio *transmitter, const ISignal *signal, double range) const
{
    double radius = range + refillPeriod * maxSpeed;
    Coord transmitterPos = transmitter->getAntenna()->getMobility()->getCurrentPosition();
    QuadTreeNeighborCacheVisitor visitor(radioMedium, transmitter, signal);
    quadTree->rangeQuery(transmitterPos, radius, &visitor);
}

void QuadTreeNeighborCache::fillQuadTreeWithRadios()
{
    for (auto & elem : radios) {
        Coord radioPos = elem->getAntenna()->getMobility()->getCurrentPosition();
        if (!quadTree->insert(check_and_cast<const cObject *>(elem), radioPos))
            throw cRuntimeError("Unsuccessful QuadTree building");
    }
}

void QuadTreeNeighborCache::rebuildQuadTree()
{
    delete quadTree;
    quadTree = new QuadTree(constraintAreaMin, constraintAreaMax, maxNumOfPointsPerQuadrant, nullptr);
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
        radioMedium->sendToRadio(transmitter, neighbor, signal);
}

} // namespace physicallayer
} // namespace inet

