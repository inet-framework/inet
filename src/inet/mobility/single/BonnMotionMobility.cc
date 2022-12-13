//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/mobility/single/BonnMotionMobility.h"

#include "inet/common/INETMath.h"
#include "inet/mobility/single/BonnMotionFileCache.h"

namespace inet {

Define_Module(BonnMotionMobility);

void BonnMotionMobility::computeMaxSpeed()
{
    const BonnMotionFile::Line& vec = *lines;
    double lastTime = vec[0];
    Coord lastPos(vec[1], vec[2], (is3D ? vec[3] : 0));
    unsigned int step = (is3D ? 4 : 3);
    for (unsigned int i = step; i < vec.size(); i += step) {
        double elapsedTime = vec[i] - lastTime;
        Coord currPos(vec[i + 1], vec[i + 2], (is3D ? vec[i + 3] : 0));
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

void BonnMotionMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);

    EV_TRACE << "initializing BonnMotionMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        is3D = par("is3D");
        int nodeId = par("nodeId");
        if (nodeId == -1)
            nodeId = getContainingNode(this)->getIndex();
        const char *fname = par("traceFile");
        const BonnMotionFile *bmFile = cache.getFile(fname);
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

