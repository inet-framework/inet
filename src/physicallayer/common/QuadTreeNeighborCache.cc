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

#include "QuadTreeNeighborCache.h"

namespace inet {

namespace physicallayer {
Define_Module(QuadTreeNeighborCache);

QuadTreeNeighborCache::QuadTreeNeighborCache()
{
    quadTree = NULL;
    radioMedium = NULL;
    rebuildQuadTreeTimer = NULL;
}

void QuadTreeNeighborCache::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        radioMedium = check_and_cast<RadioMedium *>(getParentModule());
        rebuildQuadTreeTimer = new cMessage("rebuildQuadTreeTimer");
        constraintAreaMax.x = par("constraintAreaMaxX");
        constraintAreaMax.y = par("constraintAreaMaxY");
        constraintAreaMin.x = par("constraintAreaMinX");
        constraintAreaMin.y = par("constraintAreaMinY");
        range = par("range");
        rebuildPeriod = par("refillPeriod");
        maxSpeed = par("maxSpeed");
        maxNumOfPointsPerQuadrant = par("maxNumOfPointsPerQuadrant");
        quadTree = new QuadTree(constraintAreaMin, constraintAreaMax, maxNumOfPointsPerQuadrant, NULL);
    }
    else if (stage == INITSTAGE_LINK_LAYER_2) {
        if (maxSpeed != 0)
            scheduleAt(simTime() + rebuildPeriod, rebuildQuadTreeTimer);
    }
}

void QuadTreeNeighborCache::handleMessage(cMessage *msg)
{
    if (!msg->isSelfMessage())
        throw cRuntimeError("This module only handles self messages");
    delete quadTree;
    quadTree = new QuadTree(constraintAreaMin, constraintAreaMax, maxNumOfPointsPerQuadrant, NULL);
    fillQuadTreeWithRadios();
    scheduleAt(simTime() + rebuildPeriod, msg);
}

void QuadTreeNeighborCache::addRadio(const IRadio *radio)
{
    radios.push_back(radio);
    Coord radioPos = radio->getAntenna()->getMobility()->getCurrentPosition();
    quadTree->insert(check_and_cast<const cObject *>(radio), radioPos);
}

void QuadTreeNeighborCache::removeRadio(const IRadio *radio)
{
    Radios::iterator it = find(radios.begin(), radios.end(), radio);
    if (it != radios.end()) {
        radios.erase(it);
        quadTree->remove(check_and_cast<const cObject *>(radio));
    }
    else
        throw cRuntimeError("You can't remove radio: %d because it is not in our radio container", radio->getId());
}

void QuadTreeNeighborCache::sendToNeighbors(IRadio *transmitter, const IRadioFrame *frame)
{
    double radius = range + rebuildPeriod * maxSpeed;
    Coord transmitterPos = transmitter->getAntenna()->getMobility()->getCurrentPosition();
    QuadTreeVisitor *visitor = new QuadTreeNeighborCacheVisitor(radioMedium, transmitter, frame);
    quadTree->rangeQuery(transmitterPos, radius, visitor);
    delete visitor;
}

void QuadTreeNeighborCache::fillQuadTreeWithRadios()
{
    for (unsigned int i = 0; i < radios.size(); i++) {
        Coord radioPos = radios[i]->getAntenna()->getMobility()->getCurrentPosition();
        if (!quadTree->insert(check_and_cast<const cObject *>(radios[i]), radioPos))
            throw cRuntimeError("Unsuccessful QuadTree building");
    }
}

QuadTreeNeighborCache::~QuadTreeNeighborCache()
{
    delete quadTree;
    cancelAndDelete(rebuildQuadTreeTimer);
}

void QuadTreeNeighborCache::QuadTreeNeighborCacheVisitor::visitor(const cObject *radio)
{
    radioMedium->sendToRadio(transmitter, check_and_cast<const IRadio *>(radio), frame);
}

} // namespace physicallayer
} // namespace inet

