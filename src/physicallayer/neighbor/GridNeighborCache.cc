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

#include "GridNeighborCache.h"

namespace inet {

namespace physicallayer {

Define_Module(GridNeighborCache);

void GridNeighborCache::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        // TODO: NED parameter?
        radioMedium = check_and_cast<RadioMedium *>(getParentModule());
        cellSize.x = par("cellSizeX");
        cellSize.y = par("cellSizeY");
        cellSize.z = par("cellSizeZ");
        refillPeriod = par("refillPeriod");
        init();
    }
    else if (stage == INITSTAGE_LINK_LAYER_2) {    // TODO: is it the correct stage to do this?
        fillCubeVector();
        refillCellsTimer = new cMessage("refillCellsTimer");
        maxSpeed = radioMedium->getMaxSpeed().get();
        constraintAreaMax = radioMedium->getConstraintAreaMax();
        constraintAreaMin = radioMedium->getConstraintAreaMin();
        if (maxSpeed != 0)
            scheduleAt(simTime() + refillPeriod, refillCellsTimer);
    }
}

void GridNeighborCache::fillCubeVector()
{
    for (unsigned int i = 0; i < numberOfCells; i++)
        grid[i].clear();

    for (unsigned int i = 0; i < radios.size(); i++) {
        const IRadio *radio = radios[i];
        Coord radioPos = radio->getAntenna()->getMobility()->getCurrentPosition();
        grid[posToCubeId(radioPos)].push_back(radio);
    }
}

unsigned int GridNeighborCache::posToCubeId(Coord pos)
{
    // map coordinates to indices

    unsigned int xIndex = pos.x * dimension[0] / sideLengths.x;
    unsigned int yIndex = pos.y * dimension[1] / sideLengths.y;
    unsigned int zIndex = pos.z * dimension[2] / sideLengths.z;

    return rowmajorIndex(xIndex, yIndex, zIndex);
}

unsigned int GridNeighborCache::rowmajorIndex(unsigned int xIndex, unsigned int yIndex, unsigned int zIndex)
{
    unsigned int coord[3] = {
        xIndex, yIndex, zIndex
    };
    unsigned int ind = 0;

    for (unsigned int k = 0; k < 3; k++) {
        unsigned int prodDim = 1;

        for (unsigned int l = k + 1; l < 3; l++)
            if (dimension[l] > 0)
                prodDim *= dimension[l];


        ind += prodDim * coord[k];
    }

    return ind;
}

Coord GridNeighborCache::decodeRowmajorIndex(unsigned int ind)
{
    int coord[3] = {
        -1, -1, -1
    };
    for (unsigned int k = 0; k < 3; k++) {
        unsigned int prodDim = 1;

        for (unsigned int l = k + 1; l < 3; l++)
            if (dimension[l] > 0)
                prodDim *= dimension[l];


        coord[k] = ind / prodDim;
        ind %= prodDim;
    }
    return Coord(coord[0], coord[1], coord[2]);
}

void GridNeighborCache::handleMessage(cMessage *msg)
{
    if (!msg->isSelfMessage())
        throw cRuntimeError("This module only handles self messages");

    EV_DETAIL << "Updating the grid cells" << endl;

    fillCubeVector();

    scheduleAt(simTime() + refillPeriod, msg);
}

void GridNeighborCache::addRadio(const IRadio *radio)
{
    radios.push_back(radio);
    Coord radioPos = radio->getAntenna()->getMobility()->getCurrentPosition();
    Coord newConstraintAreaMin = radioMedium->getConstraintAreaMin();
    Coord newConstraintAreaMax = radioMedium->getConstraintAreaMax();
    // If the constraintArea changed we must rebuild the grid
    if (newConstraintAreaMin != constraintAreaMin || newConstraintAreaMax != constraintAreaMax)
    {
        constraintAreaMin = newConstraintAreaMin;
        constraintAreaMax = newConstraintAreaMax;
        fillCubeVector();
    }
    else
        grid[posToCubeId(radioPos)].push_back(radio);
    maxSpeed = radioMedium->getMaxSpeed().get();
    if (maxSpeed != 0 && !refillCellsTimer->isScheduled())
        scheduleAt(simTime() + refillPeriod, refillCellsTimer);
}

void GridNeighborCache::removeRadio(const IRadio *radio)
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
        }
        maxSpeed = radioMedium->getMaxSpeed().get();
        fillCubeVector();
        if (maxSpeed == 0)
            cancelAndDelete(refillCellsTimer);
    }
    else {
        throw cRuntimeError("You can't remove radio: %d because it is not in our radio vector", radio->getId());
    }
}

void GridNeighborCache::sendToNeighbors(IRadio *transmitter, const IRadioFrame *frame, double range)
{
    double radius = range + (maxSpeed * refillPeriod);
    Coord transmitterPos = transmitter->getAntenna()->getMobility()->getCurrentPosition();

    // we have to measure the radius in cells
    int xCells = sideLengths.x == 0 ? 0 : ceil((radius * dimension[0]) / sideLengths.x);
    int yCells = sideLengths.y == 0 ? 0 : ceil((radius * dimension[1]) / sideLengths.y);
    int zCells = sideLengths.z == 0 ? 0 : ceil((radius * dimension[2]) / sideLengths.z);

    // decode the row-major index to matrix indices
    // for easier calculations
    Coord transmitterMatCoord = decodeRowmajorIndex(posToCubeId(transmitterPos));

    int transmitterMatICoord = transmitterMatCoord.x;
    int transmitterMatJCoord = transmitterMatCoord.y;
    int transmitterMatKCoord = transmitterMatCoord.z;

    // the start and the end positions of the smallest rectangle which contains our ball with
    // the radius calculated above
    int iStart = transmitterMatICoord - xCells < 0 ? 0 : transmitterMatICoord - xCells;
    int iEnd = transmitterMatICoord + xCells >= dimension[0] ? dimension[0] - 1 : transmitterMatICoord + xCells;

    int jStart = transmitterMatJCoord - yCells < 0 ? 0 : transmitterMatJCoord - yCells;
    int jEnd = transmitterMatJCoord + yCells >= dimension[1] ? dimension[1] - 1 : transmitterMatJCoord + yCells;

    int kStart = transmitterMatKCoord - zCells < 0 ? 0 : transmitterMatKCoord - zCells;
    int kEnd = transmitterMatKCoord + zCells >= dimension[2] ? dimension[2] - 1 : transmitterMatKCoord + zCells;

    // dimension[X] - 1 equals to 1 if dimension[X] = 0
    if (iEnd < 0)
        iEnd = 0;
    if (jEnd < 0)
        jEnd = 0;
    if (kEnd < 0)
        kEnd = 0;

    // sending frame to the neighbor cells

    for (int i = iStart; i <= iEnd; i++) {
        for (int j = jStart; j <= jEnd; j++) {
            for (int k = kStart; k <= kEnd; k++) {
                int cellID = rowmajorIndex(i, j, k);
                Radios& neighborCube = grid[cellID];
                unsigned int cellSize = neighborCube.size();

                for (unsigned int l = 0; l < cellSize; l++)
                    radioMedium->sendToRadio(transmitter, neighborCube[l], frame);
            }
        }
    }
}

void GridNeighborCache::calculateDimension(int *dim)
{
    int xDim = sideLengths.x / cellSize.x;
    int yDim = sideLengths.y / cellSize.y;
    int zDim = sideLengths.z / cellSize.z;

    dim[0] = xDim;
    dim[1] = yDim;
    dim[2] = zDim;
}

Coord GridNeighborCache::calculateSideLength()
{
    return Coord(constraintAreaMax.x - constraintAreaMin.x, constraintAreaMax.y - constraintAreaMin.y,
            constraintAreaMax.z - constraintAreaMin.z);
}

unsigned int GridNeighborCache::calculateNumberOfCells()
{
    unsigned int prodDim = 1;
    for (int i = 0; i < 3; i++)
        if (dimension[i] != 0)
            prodDim *= dimension[i];


    return prodDim;
}

void GridNeighborCache::init()
{
    sideLengths = calculateSideLength();
    calculateDimension(dimension);
    numberOfCells = calculateNumberOfCells();
    grid.resize(numberOfCells);
}

GridNeighborCache::~GridNeighborCache()
{
    cancelAndDelete(refillCellsTimer);
}

} // namespace physicallayer

} // namespace inet

