//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/mobility/single/BonnMotionMobility.h"
#include "inet/mobility/single/BonnMotionFileCache.h"
#include "inet/common/INETMath.h"

namespace inet {

Define_Module(BonnMotionMobility);

BonnMotionMobility::BonnMotionMobility()
{
    is3D = false;
    lines = NULL;
    currentLine = -1;
    maxSpeed = 0;
}

void BonnMotionMobility::computeMaxSpeed()
{
    const BonnMotionFile::Line& vec = *lines;
    double lastTime = vec[0];
    Coord lastPos(vec[1],vec[2],(is3D ? vec[3] : 0));
    unsigned int step = (is3D ? 4: 3);
    for (unsigned int i = step; i < vec.size(); i += step)
    {
        double elapsedTime = vec[i] - lastTime;
        Coord currPos(vec[i+1], vec[i+2], (is3D ? vec[i+3] : 0));
        double distance = currPos.distance(lastPos);
        double speed = distance / elapsedTime;
        if (speed > maxSpeed)
            maxSpeed = speed;
        lastPos.x = currPos.x;
        lastPos.y = currPos.y;
        lastPos.z = currPos.z;
        lastTime = vec[i];
    }
}

BonnMotionMobility::~BonnMotionMobility()
{
    BonnMotionFileCache::deleteInstance();
}

void BonnMotionMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);

    EV_TRACE << "initializing BonnMotionMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        is3D = par("is3D").boolValue();
        int nodeId = par("nodeId");
        if (nodeId == -1)
            nodeId = getContainingNode(this)->getIndex();
        const char *fname = par("traceFile");
        const BonnMotionFile *bmFile = BonnMotionFileCache::getInstance()->getFile(fname);
        lines = bmFile->getLine(nodeId);
        if (!lines)
            throw cRuntimeError("Invalid nodeId %d -- no such line in file '%s'", nodeId, fname);
        currentLine = 0;
        computeMaxSpeed();
    }
}

void BonnMotionMobility::setInitialPosition()
{
    const BonnMotionFile::Line& vec = *lines;
    if (lines->size() >= 3) {
        lastPosition.x = vec[1];
        lastPosition.y = vec[2];
    }
}

void BonnMotionMobility::setTargetPosition()
{
    const BonnMotionFile::Line& vec = *lines;
    if (currentLine + (is3D ? 3 : 2) >= (int)vec.size()) {
        nextChange = -1;
        stationary = true;
        targetPosition = lastPosition;
        return;
    }
    nextChange = vec[currentLine];
    targetPosition.x = vec[currentLine + 1];
    targetPosition.y = vec[currentLine + 2];
    targetPosition.z = is3D ? vec[currentLine + 3] : 0;
    currentLine += (is3D ? 4 : 3);
}

void BonnMotionMobility::move()
{
    LineSegmentsMobilityBase::move();
    raiseErrorIfOutside();
}

} // namespace inet

