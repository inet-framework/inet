//
// Copyright (C) 2014 Florian Meier <florian.meier@koalo.de>
// Copyright (C) 2006 Isabel Dietrich <isabel.dietrich@informatik.uni-erlangen.de>
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

#include "inet/mobility/static/StaticConcentricMobility.h"

namespace inet {

Define_Module(StaticConcentricMobility);

void StaticConcentricMobility::setInitialPosition()
{
    unsigned int numHosts = par("numHosts");
    double distance = par("distance");

    unsigned int index = subjectModule->getIndex();

    unsigned int totalCircles = 0;
    unsigned int totalNodesOnInnerCircles = 0;
    unsigned int nodesOnThisCircle = 1;

    unsigned int myCircle = 0;
    unsigned int nodesOnInnerCircles = 0;

    for (unsigned int i = 0; i < numHosts; i++) {
        if (i - totalNodesOnInnerCircles >= nodesOnThisCircle) {
            // start new circle
            totalCircles++;
            totalNodesOnInnerCircles += nodesOnThisCircle;
            nodesOnThisCircle = (int)(2 * M_PI * totalCircles);
        }

        if (i == index) {
            myCircle = totalCircles;
            nodesOnInnerCircles = totalNodesOnInnerCircles;
        }
    }

    lastPosition.x = distance * totalCircles;
    lastPosition.y = distance * totalCircles;

    if (constraintAreaMin.x > -INFINITY) {
        lastPosition.x += constraintAreaMin.x;
    }

    if (constraintAreaMin.y > -INFINITY) {
        lastPosition.y += constraintAreaMin.y;
    }

    if (index > 0) {
        double radius = distance * myCircle;
        double angularStep = 2.0 * M_PI / (int)(2 * M_PI * myCircle);
        double angle = angularStep * (index - nodesOnInnerCircles);
        lastPosition.x += radius * cos(angle);
        lastPosition.y += radius * sin(angle);
    }

    lastPosition.z = par("initialZ");
    recordScalar("x", lastPosition.x);
    recordScalar("y", lastPosition.y);
    recordScalar("z", lastPosition.z);
}

} // namespace inet

