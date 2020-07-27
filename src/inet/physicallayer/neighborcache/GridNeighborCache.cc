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
#include "inet/physicallayer/neighborcache/GridNeighborCache.h"

namespace inet {
namespace physicallayer {

Define_Module(GridNeighborCache);

GridNeighborCache::GridNeighborCache() :
    grid(nullptr),
    radioMedium(nullptr),
    constraintAreaMin(Coord::NIL),
    constraintAreaMax(Coord::NIL),
    refillCellsTimer(nullptr),
    refillPeriod(NaN),
    maxSpeed(NaN),
    cellSize(Coord::NIL)
{
}

void GridNeighborCache::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        radioMedium = getModuleFromPar<RadioMedium>(par("radioMediumModule"), this);
        cellSize.x = par("cellSizeX");
        cellSize.y = par("cellSizeY");
        cellSize.z = par("cellSizeZ");
        refillPeriod = par("refillPeriod");
        refillCellsTimer = new cMessage("refillCellsTimer");
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER_NEIGHBOR_CACHE) {
        constraintAreaMin = radioMedium->getMediumLimitCache()->getMinConstraintArea();
        constraintAreaMax = radioMedium->getMediumLimitCache()->getMaxConstraintArea();
        maxSpeed = radioMedium->getMediumLimitCache()->getMaxSpeed().get();
        const Coord constraintAreaSize = constraintAreaMax - constraintAreaMin;
        if (std::isnan(cellSize.x))
            cellSize.x = constraintAreaSize.x / par("cellCountX").doubleValue();
        if (std::isnan(cellSize.y))
            cellSize.y = constraintAreaSize.y / par("cellCountY").doubleValue();
        if (std::isnan(cellSize.z))
            cellSize.z = constraintAreaSize.z / par("cellCountZ").doubleValue();
        fillCubeVector();
        scheduleAfter(refillPeriod, refillCellsTimer);
    }
}

void GridNeighborCache::handleMessage(cMessage *msg)
{
    if (!msg->isSelfMessage())
        throw cRuntimeError("This module only handles self messages");
    EV_DETAIL << "Updating the grid cells" << endl;
    fillCubeVector();
    scheduleAfter(refillPeriod, msg);
}

std::ostream& GridNeighborCache::printToStream(std::ostream& stream, int level) const
{
    stream << "GridNeighborCache";
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", cellSize = " << cellSize
               << ", refillPeriod = " << refillPeriod
               << ", maxSpeed = " << maxSpeed;
    return stream;
}

void GridNeighborCache::fillCubeVector()
{
    delete grid;
    grid = new SpatialGrid(cellSize, constraintAreaMin, constraintAreaMax);
    for (auto & elem : radios) {
        const IRadio *radio = elem;
        Coord radioPos = radio->getAntenna()->getMobility()->getCurrentPosition();
        grid->insertPoint(check_and_cast<const cObject *>(radio), radioPos);
    }
}

void GridNeighborCache::addRadio(const IRadio *radio)
{
    radios.push_back(radio);
    Coord radioPos = radio->getAntenna()->getMobility()->getCurrentPosition();
    maxSpeed = radioMedium->getMediumLimitCache()->getMaxSpeed().get();
    if (maxSpeed != 0 && !refillCellsTimer->isScheduled() && initialized())
        scheduleAfter(refillPeriod, refillCellsTimer);
    Coord newConstraintAreaMin = radioMedium->getMediumLimitCache()->getMinConstraintArea();
    Coord newConstraintAreaMax = radioMedium->getMediumLimitCache()->getMaxConstraintArea();
    // If the constraintArea changed we must rebuild the grid
    if (newConstraintAreaMin != constraintAreaMin || newConstraintAreaMax != constraintAreaMax)
    {
        constraintAreaMin = newConstraintAreaMin;
        constraintAreaMax = newConstraintAreaMax;
        if (initialized())
            fillCubeVector();
    }
    else if(initialized())
        grid->insertPoint(check_and_cast<const cObject *>(radio),radioPos);
}

void GridNeighborCache::removeRadio(const IRadio *radio)
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
        }
        maxSpeed = radioMedium->getMediumLimitCache()->getMaxSpeed().get();
        fillCubeVector();
        if (maxSpeed == 0 && initialized())
            cancelEvent(refillCellsTimer);
    }
    else {
        throw cRuntimeError("You can't remove radio: %d because it is not in our radio vector", radio->getId());
    }
}

void GridNeighborCache::sendToNeighbors(IRadio *transmitter, const IWirelessSignal *signal, double range) const
{
    double radius = range + (maxSpeed * refillPeriod);
    Coord transmitterPos = transmitter->getAntenna()->getMobility()->getCurrentPosition();
    GridNeighborCacheVisitor visitor(radioMedium, transmitter, signal);
    grid->rangeQuery(transmitterPos, radius, &visitor);
}

void GridNeighborCache::GridNeighborCacheVisitor::visit(const cObject *radio) const
{
    const IRadio *neighbor = check_and_cast<const IRadio *>(radio);
    if (transmitter->getId() != neighbor->getId())
        radioMedium->sendToRadio(transmitter, neighbor, signal);
}

GridNeighborCache::~GridNeighborCache()
{
    cancelAndDelete(refillCellsTimer);
    delete grid;
}

} // namespace physicallayer
} // namespace inet

